# Maple VM-Only Consolidation and Language Surface Upgrade (Design v0.2)

Status: Completed (implemented; closeout verified on 2026-03-21)
Date: 2026-03-15
Author perspective: Programming language design and runtime engineering

## 1. Scope and Goals

This design keeps the same five mandatory goals:

1. Remove legacy interpreter mode.
2. Keep VM (bytecode) as the only execution mode.
3. Upgrade GC to a professional runtime ownership model.
4. Add a necessary standard library baseline.
5. Remove explicit `;` and use newline-based statement boundaries (Go-like style).

The document defines implementation strategy with migration safety based on the current codebase.

## 2. Current-State Analysis (Evidence from Existing Code)

## 2.1 Interpreter Path Is De-Prioritized but Still Present

Observed facts:

1. Legacy interpreter build is gated by `MAPLE_ENABLE_LEGACY_INTERPRETER` and defaults to `OFF` (`CMakeLists.txt`).
2. VM source execution still keeps legacy branches under `#if MAPLE_ENABLE_LEGACY_INTERPRETER` (`src/runtime/vm.cc`, `execute_source_named`).
3. The legacy interpreter source remains a large standalone lexer/parser/resolver/executor implementation.

Conclusion:

1. Runtime is VM-first in default builds.
2. Architectural debt remains because dual-engine code still exists.

## 2.2 VM Is Already the Mainline

Observed facts:

1. `Vm::source_mode_` defaults to `SourceExecutionMode::kVmPreferred` (`src/runtime/vm.hh`).
2. Module execution forces VM mode (`src/runtime/vm.cc`, `execute_module`).
3. Compiler/VM already cover closures, classes/inheritance, module import, and control-flow/operators.

Conclusion:

1. The migration crossing point has been reached.
2. VM-only convergence is now consolidation, not rewrite.

## 2.3 GC Works but Ownership Is Not Yet VM-Native

Observed facts:

1. `GcController` tracks pointer keys and marks/sweeps allocation entries (`src/runtime/gc.hh`, `src/runtime/gc.cc`).
2. Runtime object graph still relies on `std::shared_ptr` ownership (`src/runtime/object.hh`, `src/runtime/vm.cc`).
3. Current sweep updates GC bookkeeping, while object lifetime is still mostly shared-ownership-driven.

Conclusion:

1. Current GC provides useful tracing/accounting.
2. It is not yet a full VM-owned heap reclamation model.
3. Clox-style target requires explicit VM heap ownership + tracing/sweep reclamation.

## 2.4 Standard Library Surface Is Minimal

Observed facts:

1. No dedicated native-function registration framework exists in the VM object model.
2. Module loading currently resolves user `.ms` files (`ModuleLoader::resolve_path`).
3. Existing scripts do not depend on a formal `std.*` library contract.

Conclusion:

1. Runtime capability is usable but library surface is weak.
2. A stable standard-library baseline is required.

## 2.5 Grammar and Compiler Are Semicolon-Centric

Observed facts:

1. Lexer includes `kSemicolon` and emits it for `;` (`src/frontend/token.hh`, `src/frontend/lexer.cc`).
2. Compiler parser repeatedly uses `consume(TokenType::kSemicolon, ...)` (`src/frontend/compiler.cc`).
3. Spec grammar currently requires semicolons (`docs/spec/grammar.ebnf`).

Conclusion:

1. Removing `;` is a cross-layer language change.
2. Lexer/parser/compiler/tests/spec must be updated together.

## 3. Target Architecture Decisions

## 3.1 Execution Model Decision

Decision:

1. Remove legacy interpreter execution from production build and runtime API behavior.
2. Remove fallback route values and mode combinations that imply dual execution.

Design consequences:

1. `SourceExecutionMode` collapses to VM-only policy (or is removed).
2. `SourceExecutionRoute` can be simplified to one normative route plus diagnostics metadata.

## 3.2 GC Decision

Decision:

1. Adopt VM-owned heap objects with tracing mark-sweep as the actual ownership mechanism.

Required model shift:

1. Add object header metadata (for example: type tag, mark bit, next pointer).
2. Allocate runtime heap objects via VM allocation API, not `shared_ptr` ownership.
3. Sweep phase must reclaim raw allocations directly.

Note:

1. Keep and extend `GcStats` observability.
2. Keep stop-the-world mark-sweep in v1 (no generational/incremental design yet).

## 3.3 Standard Library Decision

Decision:

1. Provide a minimal stable standard library in two layers:
   - VM native builtins
   - source-level `std.*` modules

Recommended v1 API set:

1. `std.io.print(value)`, `std.io.println(value)`
2. `std.math.abs(x)`, `std.math.min(a,b)`, `std.math.max(a,b)`, `std.math.floor(x)`, `std.math.ceil(x)`
3. `std.str.len(s)`, `std.str.substr(s, start, len)`, `std.str.contains(s, t)`
4. `std.time.now_unix_ms()`
5. `std.debug.typeof(value)`

## 3.4 Newline-Terminated Statement Decision

Decision:

1. Introduce newline-sensitive parsing with newline token + statement-terminator abstraction.
2. Keep semicolon compatibility for one short migration window, then remove it.

Why:

1. Removing `;` without newline tokenization creates boundary ambiguity.
2. Go-like behavior requires lexical insertion rules instead of parser-only heuristics.

## 4. Implementation Plan

## Phase P0: Spec First and Contract Lock

Changes:

1. Update `docs/spec/grammar.ebnf` to newline-terminated productions.
2. Update `docs/spec/semantics.md` examples and statement-boundary rules.
3. Add newline/terminator error-code mapping in `docs/spec/errors.md`.

Exit criteria:

1. VM-only + newline statement contracts are internally consistent in spec.

## Phase P1: Remove Legacy Interpreter Mode

Changes:

1. Remove the legacy interpreter interface and source files.
2. Remove `MAPLE_ENABLE_LEGACY_INTERPRETER` from `CMakeLists.txt`.
3. Keep `Vm::execute_source_named` on a single compile->VM path.
4. Simplify mode/route enums and related tests.

Exit criteria:

1. No runtime branch invokes interpreter.
2. CI has no fallback-dependent assertions.

## Phase P2: Professional GC Core

Changes:

1. Refactor object representation from shared-ownership graph semantics to VM-managed heap allocations.
2. Introduce object allocator APIs and centralized object-list maintenance.
3. Complete root tracing: stack, call frames, open upvalues, globals, module cache/exports, compiler temporary roots.
4. Complete raw sweep reclamation and keep intern/cache structures consistent.

Exit criteria:

1. Object lifecycle no longer depends on `shared_ptr` reachability.
2. GC stress tests show stable reclamation without semantic regression.

## Phase P3: Standard Library Baseline

Changes:

1. Introduce native callable object kind and VM dispatch support.
2. Register core natives at VM initialization.
3. Add `std` bootstrap policy (eager cache registration or lazy synthetic loading).
4. Add conformance/integration tests for each std API.

Exit criteria:

1. `import std.io/std.math/std.str/std.time/std.debug` is stable.
2. API contracts and diagnostics are documented in spec.

## Phase P4: Newline Statement Terminators

Changes:

1. Lexer emits meaningful `kNewline` tokens and keeps diagnostics metadata.
2. Parser/compiler replace semicolon consumption with `consume_statement_end`.
3. Statement end supports `kNewline`, `kRightBrace`, or EOF contextually.
4. Keep braces and parenthesized `if/while/for` conditions in initial migration.
5. Accept optional `;` with deprecation warning for one cycle, then remove semicolon token/dependencies.

Exit criteria:

1. Scripts can be migrated to newline style.
2. Parser ambiguity and diagnostics remain stable under conformance tests.

## 5. Test Strategy and Acceptance Gates

## 5.1 Required Test Families

1. VM-only architecture tests (no legacy symbols/branches).
2. GC ownership tests (closure/class/module/string churn + forced GC assertions).
3. Standard-library tests (API success and error paths).
4. Newline syntax tests (single-line statements, multiline continuation, block/EOF boundaries).

## 5.2 CI Gates

1. Build must pass with VM-only configuration.
2. Unit/integration/conformance/diagnostics suites must pass.
3. Add guard test to reject legacy interpreter reintroduction.

## 6. Risks and Mitigations

Risk 1: GC refactor may introduce memory-safety regressions.
Mitigation:

1. Land object headers/allocator before semantic changes.
2. Add stress tests before removing shared-ownership paths.

Risk 2: Newline rules may introduce parser ambiguities.
Mitigation:

1. Use explicit newline tokenization + grammar-level statement-end rules.
2. Keep braces and parenthesized control headers in first migration stage.

Risk 3: Standard library scope may expand uncontrollably.
Mitigation:

1. Freeze v1 API set.
2. Require spec and tests before new std APIs.

Risk 4: Semicolon removal may break existing scripts abruptly.
Mitigation:

1. Keep one transition cycle with optional semicolon acceptance.
2. Provide migration formatter/linter rules in `tests/scripts` tooling.

## 7. Suggested Execution Order

1. P0 spec lock.
2. P1 interpreter retirement.
3. P4 newline parser foundation (transition mode).
4. P3 standard-library baseline.
5. P2 professional GC core (partially parallelizable, but finalized after syntax stabilization).

Rationale:

1. Remove architecture ambiguity first.
2. Stabilize language surface second.
3. Complete deep runtime ownership refactor last.

## 8. Deliverables Checklist

1. Updated spec docs (`grammar`, `semantics`, `errors`).
2. VM-only runtime code (legacy interpreter removed).
3. Professional GC ownership model implementation.
4. Standard library v1 modules and native bindings.
5. Newline statement parser/compiler support.
6. Full CI pass with updated conformance suite.

## 9. Closeout Evidence

Closeout date: 2026-03-21.

Build evidence:

1. `cmake -S . -B build` completed successfully.
2. `cmake --build build --config Debug` completed successfully and produced `maple_core`, `maple_cli`, and `maple_tests`.

CTest evidence:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_unit` passed.
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration` passed.
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_migration_debt` passed.
4. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance` passed.
5. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics` passed.

Script evidence:

1. `tests/scripts/migration/m6_vm_only_convergence.ms` executed with exit code `0`.
2. All scripts under `tests/scripts/migration/newline/` executed with exit code `0`.
3. Std success scripts (`io_math_ok.ms`, `str_time_debug_ok.ms`) executed with exit code `0`.
4. Std error scripts (`debug_arity_error.ms`, `io_arity_error.ms`, `math_type_error.ms`, `str_type_error.ms`, `time_arity_error.ms`) executed with expected runtime diagnostics and exit code `2`.

Document closeout:

1. `docs/improve_02_task.md` updated with final task status and per-task evidence.
2. `docs/spec/grammar.ebnf`, `docs/spec/semantics.md`, and `docs/spec/errors.md` status annotations updated from draft planning wording to implemented baseline wording.