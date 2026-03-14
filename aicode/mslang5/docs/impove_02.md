# Maple VM-Only Consolidation and Language Surface Upgrade (Design v0.2)

Status: Proposal  
Date: 2026-03-15  
Author perspective: Programming language design and runtime engineering

## 1. Scope and Goals

This design targets five mandatory changes:

1. Remove the legacy interpreter mode.
2. Keep only VM (bytecode) execution mode.
3. Upgrade GC to a more professional implementation.
4. Add a necessary standard library baseline.
5. Remove explicit `;` statement terminators and use newline-based statement boundaries (Go-style user experience).

This document defines an implementation strategy with migration safety based on the current codebase.

## 2. Current-State Analysis (Evidence from Existing Code)

## 2.1 Interpreter Path Is Already De-Prioritized but Not Removed

Observed facts:

1. Legacy interpreter compilation is gated by CMake option:
   - `MAPLE_ENABLE_LEGACY_INTERPRETER` defaults to `OFF` (`CMakeLists.txt`).
2. VM source execution still keeps legacy hooks behind compile-time guards:
   - `src/runtime/vm.cc` contains `#if MAPLE_ENABLE_LEGACY_INTERPRETER` branches in `execute_source_named`.
3. Legacy implementation is still a large standalone unit:
   - `src/runtime/script_interpreter.cc` includes lexer/parser/resolver/executor logic.

Conclusion:

1. Runtime is VM-first by default.
2. Architectural debt remains because a second engine is still in-tree.

## 2.2 VM Is the Real Mainline Already

Observed facts:

1. `Vm::source_mode_` defaults to `SourceExecutionMode::kVmPreferred` (`src/runtime/vm.hh`).
2. Module execution forces VM mode (`src/runtime/vm.cc`, `execute_module`).
3. Current compiler/VM already support:
   - closures/upvalues
   - class/inheritance/`this`/`super`
   - module import/from-import
   - control flow and expression operators

Conclusion:

1. Migration is already past the core threshold.
2. VM-only convergence is now cleanup and hardening, not a rewrite.

## 2.3 GC Is Functional but Not Yet Professional-Grade Runtime Ownership

Observed facts:

1. `GcController` tracks allocations by raw-pointer keys and marks/sweeps map entries (`src/runtime/gc.hh`, `src/runtime/gc.cc`).
2. Runtime objects are still owned via `std::shared_ptr` in VM/object graph (`src/runtime/object.hh`, `src/runtime/vm.cc`).
3. Sweep removes GC bookkeeping entries, but object lifetime is still mainly driven by shared ownership.

Conclusion:

1. Current GC is a useful tracing/accounting layer.
2. It is not yet a canonical VM-owned heap with deterministic reclamation.
3. For a clox-style runtime, ownership should move from `shared_ptr` graph semantics to explicit VM heap tracing.

## 2.4 Standard Library Surface Is Minimal

Observed facts:

1. There is no explicit native-function registration framework in the VM object model.
2. Built-in module loading currently targets user files (`ModuleLoader::resolve_path` -> filesystem `.ms` modules).
3. Existing scripts rely on language features, not a formal standard-library namespace.

Conclusion:

1. Runtime is usable but ecosystem-poor.
2. A stable standard-library baseline is required.

## 2.5 Grammar and Compiler Are Semicolon-Centric

Observed facts:

1. Lexer token set includes `kSemicolon`, and lexer emits it from `;` (`src/frontend/token.hh`, `src/frontend/lexer.cc`).
2. Compiler parser repeatedly requires semicolons via `consume(TokenType::kSemicolon, ...)` (`src/frontend/compiler.cc`).
3. Spec grammar requires semicolons in declarations/statements (`docs/spec/grammar.ebnf`).

Conclusion:

1. Removing `;` is a cross-layer language change, not a parser-only patch.
2. Updates are required in lexer, parser, compiler, tests, and specs.

## 3. Target Architecture Decisions

## 3.1 Execution Model Decision

Decision:

1. Delete legacy interpreter execution path from production build and runtime APIs.
2. Remove fallback route enum values and mode combinations that imply dual execution.

Design consequences:

1. `SourceExecutionMode` collapses to VM-only policy (or is removed).
2. `SourceExecutionRoute` can be simplified to one normative route plus diagnostics metadata.

## 3.2 GC Decision

Decision:

1. Move to VM-owned heap objects with tracing mark-sweep as the real ownership model.

Required model shift:

1. Introduce object header fields (for example: type tag, mark bit, next pointer).
2. Allocate runtime heap objects via VM allocation APIs, not `shared_ptr`.
3. Sweep must reclaim raw allocations directly.

Note:

1. Keep and extend current `GcStats` observability.
2. Keep stop-the-world mark-sweep in v1 (no generational/incremental complexity yet).

## 3.3 Standard Library Decision

Decision:

1. Provide a minimal stable standard-library baseline in two layers:
   - native builtins (VM-registered callables)
   - source-level modules under a reserved namespace (for example `std.*`)

Recommended v1 API set:

1. `std.io.print(value)` and `std.io.println(value)`
2. `std.math.abs(x)`, `std.math.min(a,b)`, `std.math.max(a,b)`, `std.math.floor(x)`, `std.math.ceil(x)`
3. `std.str.len(s)`, `std.str.substr(s, start, len)`, `std.str.contains(s, t)`
4. `std.time.now_unix_ms()`
5. `std.debug.typeof(value)`

## 3.4 Newline-Terminated Statement Decision

Decision:

1. Adopt explicit newline-sensitive parsing by introducing a newline token and a statement-terminator abstraction.
2. Keep optional semicolon compatibility only in a short migration window (one release window), then remove it.

Why this strategy:

1. Removing `;` without newline tokenization creates boundary ambiguity.
2. Go-like behavior needs lexical insertion rules, not ad-hoc parser heuristics.

## 4. Implementation Plan

## Phase P0: Spec First and Contract Lock

Changes:

1. Update `docs/spec/grammar.ebnf` from semicolon-required productions to newline-terminated productions.
2. Update `docs/spec/semantics.md` examples and statement-boundary rules.
3. Add error-code mappings for newline/terminator errors in `docs/spec/errors.md`.

Exit criteria:

1. Spec is internally consistent for VM-only and newline statements.

## Phase P1: Remove Legacy Interpreter Mode

Changes:

1. Remove `src/runtime/script_interpreter.hh` and `src/runtime/script_interpreter.cc` from build and source tree.
2. Remove `MAPLE_ENABLE_LEGACY_INTERPRETER` option and compile definitions from `CMakeLists.txt`.
3. Simplify `Vm::execute_source_named` to one path:
   - compile to bytecode
   - execute in VM
4. Simplify mode/route enums and related tests.

Exit criteria:

1. No runtime branch can invoke legacy interpreter code.
2. CI has no fallback-path assertions.

## Phase P2: Professional GC Core

Changes:

1. Refactor object representation:
   - replace shared-ownership graph semantics in runtime heap types with VM-managed allocations.
2. Introduce object allocator APIs:
   - `allocate_object<T>(...)`
   - centralized object-list maintenance in VM/GC subsystem.
3. Mark traversal:
   - stack values
   - call frame closures/functions
   - open upvalues
   - globals table
   - module cache and module exports
   - compiler temporary roots (when constants/closures are built)
4. Sweep reclamation:
   - free unreachable objects directly
   - keep intern tables/caches consistent

Exit criteria:

1. Object lifecycle no longer depends on `shared_ptr` reachability.
2. GC stress tests show stable reclamation with no semantic regressions.

## Phase P3: Standard Library Baseline

Changes:

1. Introduce native callable object kind and dispatch path in VM.
2. Register core natives during VM initialization.
3. Add `std` module bootstrap policy:
   - either eager registration in module cache
   - or lazy synthetic module-loading hooks.
4. Add conformance and integration tests for each standard-library API.

Exit criteria:

1. `import std.io`, `import std.math`, `import std.str`, `import std.time`, `import std.debug` are stable.
2. API contracts and diagnostics are spec-documented.

## Phase P4: Newline Statement Terminators

Changes:

1. Lexer:
   - emit `kNewline` tokens when line breaks are meaningful.
   - keep line/column tracking for diagnostics.
2. Parser/compiler:
   - replace semicolon `consume` with `consume_statement_end`.
   - support statement end by `kNewline`, `kRightBrace`, or EOF contextually.
3. Control-structure handling:
   - keep block braces for now (to minimize ambiguity).
   - keep parenthesized `if/while/for` conditions initially.
4. Transitional compatibility:
   - accept `;` as optional separator with deprecation warning for one migration cycle.
5. Final cleanup:
   - remove `kSemicolon` token and all parser semicolon requirements after transition.

Exit criteria:

1. Existing scripts can be auto-migrated to newline style.
2. Parser ambiguity and diagnostics remain stable under conformance tests.

## 5. Test Strategy and Acceptance Gates

## 5.1 New Required Test Families

1. VM-only architecture tests:
   - assert no legacy symbols/branches remain.
2. GC ownership tests:
   - churn tests for closures/classes/modules/strings.
   - forced-GC tests with object-count/liveness assertions.
3. Standard library tests:
   - API functional tests
   - error-path tests (wrong arity/type, missing module symbol)
4. Newline syntax tests:
   - one-statement-per-line normal cases
   - multiline expression continuation cases
   - block boundary and EOF cases

## 5.2 CI Gates

1. Build must pass with VM-only configuration (no optional legacy switch).
2. Full suites must pass:
   - unit
   - integration
   - conformance
   - diagnostics
3. Add a migration test that rejects re-introduction of legacy interpreter entry points.

## 6. Risks and Mitigations

Risk 1: GC refactor introduces memory-safety regressions.  
Mitigation:

1. Land object headers and allocator first without changing semantics.
2. Add stress tests before removing `shared_ptr` ownership.

Risk 2: Newline rules create parser ambiguities.  
Mitigation:

1. Add explicit newline tokenization and grammar-level statement-end rules.
2. Keep braces and parenthesized control headers in initial migration.

Risk 3: Standard library scope expands uncontrollably.  
Mitigation:

1. Freeze v1 API set to minimal core modules.
2. Require spec + tests before adding new std APIs.

Risk 4: Existing scripts break abruptly after semicolon removal.  
Mitigation:

1. Use one transitional cycle with optional semicolon acceptance.
2. Provide migration formatter/linter rules in `tests/scripts` tooling.

## 7. Suggested Execution Order

1. P0 spec lock.
2. P1 interpreter retirement.
3. P4 newline parser foundation (transitional dual acceptance).
4. P3 standard library baseline.
5. P2 professional GC core (can overlap partially, but should complete after VM syntax stabilization).

Rationale:

1. Remove architectural ambiguity first.
2. Then stabilize syntax and language surface.
3. Finally complete the deepest runtime ownership refactor with fewer moving language targets.

## 8. Deliverables Checklist

1. Updated spec docs (`grammar`, `semantics`, `errors`).
2. VM-only runtime code (legacy interpreter removed).
3. Professional GC ownership model implementation.
4. Standard library v1 modules and native bindings.
5. Newline-terminated statement parser/compiler support.
6. Full passing CI with updated conformance suite.
