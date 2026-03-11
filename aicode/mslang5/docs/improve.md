# Maple Iteration Plan Toward a Production-Grade Scripting Language

Status: Proposal  
Date: 2026-03-11  
Scope: Design and implementation roadmap only; no code implementation

## 1. Current Implementation Audit (from actual repo code)

These conclusions come from current source and tests, not only from `PLAN.md`.

### 1.1 Execution Architecture Status

1. Two execution paths exist:
   - Bytecode path: `compile_to_chunk()` -> `Vm::execute(const Chunk&)`
   - Interpreter path: `ScriptInterpreter::execute()`
2. Default VM mode is `kVmPreferredWithLegacyFallback`; compile failure falls back to interpreter.
3. Integration tests explicitly accept/depend on fallback behavior (closure/class/resolver cases assert `kVmCompileFailedThenLegacy`).
4. `ADR-001` already states VM should be the only normative path, but convergence is incomplete.

Conclusion: current architecture is transitional, not a fully VM-native language runtime.

### 1.2 Bytecode and Compiler Coverage

1. `OpCode` currently includes only basics: constants, arithmetic, globals, print, import, return.
2. `src/frontend/compiler.cc` currently covers only:
   - arithmetic expressions
   - `var`
   - global assignment
   - `import` / `from import`
3. Syntax already documented but not compiled to bytecode: if/while/for, closures, classes/inheritance, `this/super`, comparison/logical ops.

Conclusion: bytecode frontend is still a minimal subset and cannot carry full language semantics.

### 1.3 Where Semantics Actually Live

1. Closures, classes, inheritance, method binding, and resolver-like static semantics are mainly implemented in `script_interpreter.cc`.
2. Full-script behavior in `Vm` heavily depends on fallback to interpreter.
3. So semantic correctness is mostly guaranteed by interpreter behavior, not by VM-native behavior.

Conclusion: normative semantics are not yet migrated to bytecode + VM mainline.

### 1.4 Object System and GC Status

1. `RuntimeObject` is currently minimal (`to_string()` only); no mature object graph/header model.
2. `GcController` is only threshold counting + collect counter, without reachability tracing.
3. Missing parts include:
   - gray stack / mark phase
   - root set enumeration (stack, call frames, upvalues, globals, modules, compiler constants)
   - sweep reclamation over object graph

Conclusion: GC is still a placeholder, not a production mark-sweep collector.

### 1.5 Module System Status

1. `ModuleLoader` already has a base state machine (unseen/initializing/initialized/failed) and `MS5001~MS5004`.
2. But module top-level semantics are still coupled to current execution-route migration state.
3. Import is basically usable, but not yet fully controlled by a complete VM instruction-level lifecycle.

Conclusion: module system is usable but still migration-dependent.

## 2. Target Definition (done-state for a production scripting language)

Done-state definition:

1. Single normative execution path: `source -> lexer -> parser -> resolver -> bytecode compiler -> VM`.
2. `ScriptInterpreter` no longer participates in normal execution (removed, or debug/reference only and off by default).
3. Complete clox-style bytecode coverage for:
   - control flow, locals, closures, classes/inheritance, method calls, `this/super`
   - module import/export binding
4. VM supports:
   - CallFrame stack
   - upvalue capture/close lifecycle
   - object allocation and reclamation
5. GC is real tracing mark-sweep:
   - complete root tracing
   - mark / blacken / sweep
   - string interning and object table consistency
6. Conformance and diagnostics validate VM path as primary; all fallback-dependent assertions are removed.

## 3. Design Principles (mandatory during iteration)

1. Semantics first: update `docs/spec/*` before implementation.
2. One-way migration: new language features go only into VM pipeline, never interpreter.
3. Rollback-safe layering: each phase can roll back independently without breaking buildability.
4. Observability: every phase adds minimum diagnostics + regression tests.
5. Stable error contract: `phase + code` is compatibility anchor.

## 4. Target Architecture (production version)

## 4.1 Compilation Pipeline

1. Lexer: tokenization + MS1xxx.
2. Parser: syntax structure (light AST or single-pass Pratt + declaration structures).
3. Resolver: lexical depth/upvalue metadata + MS3xxx.
4. Compiler:
   - emit local/closure/global access instructions from metadata
   - produce function prototypes and constant pool
5. VM:
   - execute chunk and function chunk
   - coordinate call frames, object system, modules, and GC

## 4.2 Recommended Bytecode Instruction Layers

Organize by feature domain to avoid one-shot expansion:

1. Stack/constants: `CONSTANT`, `NIL`, `TRUE`, `FALSE`, `POP`
2. Local/global: `GET_LOCAL`, `SET_LOCAL`, `GET_GLOBAL`, `DEFINE_GLOBAL`, `SET_GLOBAL`
3. Closures: `GET_UPVALUE`, `SET_UPVALUE`, `CLOSURE`, `CLOSE_UPVALUE`
4. Ops/comparison: `ADD`, `SUBTRACT`, `MULTIPLY`, `DIVIDE`, `NEGATE`, `EQUAL`, `GREATER`, `LESS`, `NOT`
5. Control flow: `JUMP`, `JUMP_IF_FALSE`, `LOOP`, `RETURN`
6. Calls: `CALL`, `INVOKE`, `SUPER_INVOKE`
7. Class system: `CLASS`, `INHERIT`, `METHOD`, `GET_PROPERTY`, `SET_PROPERTY`, `GET_SUPER`
8. Modules: `IMPORT_MODULE`, `IMPORT_SYMBOL` (optionally later `EXPORT_SYMBOL`)

## 4.3 Recommended VM Runtime Model

1. Value:
   - keep tagged variant now (NaN-boxing can be a later optimization)
2. Object base:
   - `ObjType + marked + next`
   - subclasses: String, Function, Closure, Upvalue, Class, Instance, BoundMethod, Module
3. Call model:
   - `CallFrame{closure, ip, slots_base}`
   - slot windows to avoid argument copying
4. Upvalue:
   - open-upvalue list ordered by slot address
   - close with `CLOSE_UPVALUE` on scope exit

## 4.4 GC Design Recommendation (v1 target)

Use stop-the-world mark-sweep:

1. Allocation tracking: every object goes into object linked list.
2. Root set:
   - VM stack values
   - frame closures
   - open upvalues
   - globals table
   - module cache/exports
   - compiler temporary roots
3. Marking:
   - mark roots -> gray-stack object traversal
4. Sweep:
   - reclaim unmarked objects
   - reset mark bits
5. Trigger policy:
   - `next_gc = live_bytes * growth_factor`, replacing fixed-threshold placeholder logic
6. Debug switches:
   - GC logs (bytes before/after, reclaimed bytes, object counts)

## 5. Phased Iteration Plan (7 milestones)

Each phase must keep: buildable code, executable tests, and traceable `PLAN.md` updates with evidence.

### M0: Baseline Freeze and Migration Guardrails

Goals:

1. Mark `ScriptInterpreter` as transition-only; no new features there.
2. Build a VM capability gap matrix (syntax item -> VM support -> fallback trigger).
3. Classify fallback-related CI cases as migration debt.

DoD:

1. Unified migration checklist exists in docs.
2. New PRs cannot add language capability to interpreter path.

### M1: Frontend Capability Completion for Core Compilable Semantics

Goals:

1. Extend parser/compiler coverage:
   - block, if, while, for
   - comparison/logical/equality operators
   - locals and scope depth
2. Resolver output is consumed by compiler (not interpreter-only).

DoD:

1. Core control-flow scripts run via `kVmPipeline`.
2. Error contracts remain stable for MS2xxx/MS3xxx.

### M2: Function and Closure VM Migration

Goals:

1. Implement Function/Closure/Upvalue object lifecycle.
2. Emit `CLOSURE/GET_UPVALUE/SET_UPVALUE/CLOSE_UPVALUE/CALL/RETURN`.
3. Stabilize callframe + slot windows for recursion and higher-order functions.

DoD:

1. Closure integration cases no longer fallback.
2. Existing closure semantics stay consistent across VM and legacy (if legacy retained).

### M3: Class/Inheritance/this/super VM Migration

Goals:

1. Complete object model with `Class/Instance/BoundMethod`.
2. Compile/execute method binding, inheritance chain, super dispatch.
3. Solidify `init` semantics in VM.

DoD:

1. Class test family runs via `kVmPipeline`.
2. `MS3002/MS3003/MS3004/MS4004/MS4005` behavior matches docs.

### M4: Module System and VM Execution Protocol Unification

Goals:

1. Make module initialization strictly VM-based; define module execution context protocol.
2. Fix export binding order/scope rules (avoid partial-initialization gaps).
3. Align cache/failed-cache behavior with `docs/spec/modules.md`.

DoD:

1. Module conformance does not depend on fallback.
2. `MS5001~MS5004` are stable on VM path.

### M5: Real GC Integration

Goals:

1. Introduce full object graph + mark-sweep.
2. Connect root tracing for VM, module, globals, callframe, upvalue.
3. Remove current `GcController` placeholder collection behavior.

DoD:

1. Add GC stress tests:
   - mixed heavy closures/class instances/string interning
   - post-collection semantic correctness without dangling refs
2. GC stats are observable and match expected liveness/reclaim trends.

### M6: Interpreter Retirement and Execution-Path Convergence

Goals:

1. Switch default mode to VM-only.
2. Remove fallback-dependent test assertions.
3. Choose one strategy:
   - fully remove `ScriptInterpreter`
   - or keep debug/reference-only compile switch (default OFF)

DoD:

1. `Vm::execute_source_named` no longer calls interpreter in normal path.
2. Conformance/diagnostics/integration all pass with VM as single path.

## 6. Key Design Details and Engineering Decisions

### 6.1 Whether to Keep AST

Recommendation:

1. Keep light AST + resolver short-term to reduce migration risk.
2. Evaluate moving toward clox-style single-pass compile later; not required now.

Reason: interpreter already contains AST semantic assets; full rewrite now is high risk.

### 6.2 Unified Diagnostics Strategy

1. All phases output unified `Diagnostic`.
2. Legacy text errors remain temporary input; final rendering stays structured.
3. Conformance matches `phase + code` first, message second.

### 6.3 Module/Global Interaction

Clarify two points:

1. v1 export policy for module top-level symbols (currently all top-level symbols).
2. Boundary rule between `define_global` and `module.exports` sync during module execution.

### 6.4 GC and String/Module Caches

1. String intern table and module cache are root-sensitive containers and must be marked.
2. If failed-module metadata keeps string messages, define whether they are GC-managed (object references must always be traced).

## 7. Testing and Acceptance Strategy (during migration)

## 7.1 Strengthened Test Layers

1. Unit:
   - opcode encode/decode
   - compiler emission snapshots
   - resolver metadata
   - GC mark/sweep step behavior
2. Integration:
   - closure, class, module, CLI/REPL
3. Conformance:
   - matrix mapping from semantic rules to error codes
4. Diagnostics golden:
   - phase/code/span stability

## 7.2 Recommended Migration Metrics

1. `fallback_rate`: ratio of executed cases that trigger fallback.
2. `vm_coverage`: coverage of syntax/semantic features natively supported by VM.
3. `gc_effectiveness`: reclaimed bytes vs live-byte trend in stress tests.

Targets:

1. `fallback_rate` should drop significantly after M2.
2. Before M6 closeout, `fallback_rate == 0` in normal mode.

## 8. Major Risks and Mitigations

1. Risk: semantic drift between interpreter and VM.
   - Mitigation: dual-route parity tests before default-route switch.
2. Risk: hidden dangling references after real GC integration.
   - Mitigation: ship root-tracing observability first, then run high-frequency GC stress regressions.
3. Risk: new timing issues in module cycle handling after VM migration.
   - Mitigation: add "export visibility during initialization" case family; refine `MS5003/MS5004` boundary.
4. Risk: uncontrolled expansion from adding too many opcodes at once.
   - Mitigation: batch by M1-M4 feature domains; avoid cross-milestone mixed large changes.

## 9. Recommended Execution Order and Doc Sync

1. First update/freeze v1 behavior in:
   - `docs/spec/grammar.ebnf`
   - `docs/spec/semantics.md`
   - `docs/spec/value-model.md`
   - `docs/spec/modules.md`
   - `docs/spec/errors.md`
2. Then execute M0 -> M6; after each phase:
   - update `PLAN.md` status and evidence
   - update conformance matrix mapping
   - record fallback reduction metrics

## 10. Conclusion

Maple already has major components of a teaching-grade scripting language prototype, but still has structural gaps to reach production done-state (complete bytecode, VM, GC).  
The correct route is not adding features to interpreter; it is strict staged migration: VM semantic convergence + real GC + stronger test gates.  
Core value of this plan:

1. Fix architectural truth first (single normative path), then extend semantics.
2. Replace "claimed done" with verifiable milestones.
3. Break high-risk domains (closure/class/module/GC) into rollback-safe, observable engineering units.

## 11. Progress Tracking (for execution)

This section turns M0-M6 into an executable checklist.  
Rule: after each subsection is finished, immediately update status, date, verification evidence, and linked commit.

### 11.1 Status Definitions

1. `todo`: not started.
2. `doing`: in progress (prefer only one primary subsection doing per milestone).
3. `blocked`: blocked by dependency/risk.
4. `done`: completed with verification evidence.

### 11.2 Execution Record Template (reuse for each subsection)

```md
- Subsection: <e.g., M2-03>
  - Status: todo|doing|blocked|done
  - Owner: <name/agent>
  - Start date: YYYY-MM-DD
  - End date: YYYY-MM-DD
  - Code changes: <key file list>
  - Verification commands:
    - `cmake --build build --config Debug`
    - `ctest --test-dir build --output-on-failure -C Debug`
    - <optional: focused subset command>
  - Verification result: pass|fail
  - Evidence: <log summary/test id/screenshot path>
  - Linked commit: <commit hash / PR / branch>
  - Notes: <risk, rollback point, follow-up>
```

### 11.3 Overview Board (M0-M6)

| Milestone | Status | Progress | Start date | End date | Notes |
|---|---|---|---|---|---|
| M0 Baseline freeze and guardrails | done | 100% | 2026-03-11 | 2026-03-11 | Freeze guard + VM gap matrix + migration debt suite landed |
| M1 Frontend capability completion | done | 100% | 2026-03-12 | 2026-03-12 | VM-native control-flow/operators/locals + conformance landed |
| M2 Function/closure VM migration | todo | 0% | - | - | - |
| M3 Class/inheritance/this/super VM migration | todo | 0% | - | - | - |
| M4 Module VM protocol unification | todo | 0% | - | - | - |
| M5 Real GC integration | todo | 0% | - | - | - |
| M6 Interpreter retirement and path convergence | todo | 0% | - | - | - |

---

### 11.4 M0 Progress Subsections (baseline freeze and guardrails)

- `M0-01` Define interpreter freeze policy (no new capabilities in interpreter path)
  - Status: done
  - Done criteria: docs/review rules explicitly enforce "new language features only in VM pipeline".
  - Evidence:
    - `docs/migration/m0-baseline-freeze.md` defines freeze policy and PR checklist.
    - `cmake/check_interpreter_freeze.cmake` + `docs/migration/interpreter_freeze.sha256` enforce interpreter freeze in CI.
- `M0-02` Build VM gap matrix (syntax item -> VM support -> fallback trigger)
  - Status: done
  - Done criteria: maintainable matrix exists and maps to test cases.
  - Evidence:
    - `docs/migration/m0-baseline-freeze.md` Section 2 provides a maintainable capability matrix with representative test mappings.
- `M0-03` Mark migration debt in CI (fallback-related case classification)
  - Status: done
  - Done criteria: fallback cases have independent labels or metrics output.
  - Evidence:
    - `tests/integration/test_migration_debt.cc` tracks fallback debt cases and emits `fallback_rate`.
    - `CMakeLists.txt` adds dedicated `maple_tests_migration_debt` and `maple_guard_interpreter_freeze` with `migration_debt` label.

### 11.5 M1 Progress Subsections (frontend completion)

- `M1-01` Complete control-flow compile support (block/if/while/for)
  - Status: done
  - Done criteria: corresponding scripts execute successfully on VM path.
  - Evidence:
    - `src/frontend/compiler.cc` adds VM bytecode emission for block/if/while/for.
    - `src/runtime/vm.cc` adds `JUMP/JUMP_IF_FALSE/LOOP` execution.
    - conformance:
      - `tests/conformance/semantics/control_if_else_001.ms`
      - `tests/conformance/semantics/control_while_for_001.ms`
- `M1-02` Complete comparison/logical/equality compile + runtime behavior
  - Status: done
  - Done criteria: behavior matches `docs/spec/semantics.md`.
  - Evidence:
    - compiler emits `EQUAL/GREATER/LESS/NOT` and logical short-circuit (`and/or`) jumps.
    - VM executes `kEqual/kGreater/kLess/kNot` with runtime checks.
    - unit coverage extended in `tests/unit/test_vm_compiler.cc`.
- `M1-03` Connect resolver metadata into compiler (not interpreter-only)
  - Status: done
  - Done criteria: compiler actually consumes lexical depth metadata.
  - Evidence:
    - compiler now resolves lexical locals by scope depth/slot (`Local{name, depth}` + `resolve_local`) and emits `GET_LOCAL/SET_LOCAL`.
    - block-scope shadowing and updates execute on VM path (validated by unit + conformance).
- `M1-04` M1 verification closeout
  - Status: done
  - Done criteria: related tests run stably on `kVmPipeline`; MS2xxx/MS3xxx contract unchanged.
  - Evidence:
    - `cmake --build build --config Debug` pass.
    - `ctest --test-dir build --output-on-failure -C Debug` pass (`7/7`).
    - `ctest --test-dir build --output-on-failure -C Debug -L migration_debt` pass (`2/2`).

### 11.6 M2 Progress Subsections (function and closure VM migration)

- `M2-01` Complete object model for Function/Closure/Upvalue
  - Status: todo
  - Done criteria: runtime object structure/lifecycle covered by tests.
- `M2-02` Land instruction chain (CLOSURE/GET_UPVALUE/SET_UPVALUE/CLOSE_UPVALUE/CALL/RETURN)
  - Status: todo
  - Done criteria: compiler emit + VM execute full chain.
- `M2-03` Stabilize callframe and slot windows (recursion/higher-order functions)
  - Status: todo
  - Done criteria: recursion, closure capture, and write-back semantics are stable.
- `M2-04` Remove fallback usage (closure cases)
  - Status: todo
  - Done criteria: closure integration/parity cases no longer trigger fallback.

### 11.7 M3 Progress Subsections (class and inheritance VM migration)

- `M3-01` Complete object model for Class/Instance/BoundMethod
  - Status: todo
  - Done criteria: fields/methods/bound-method behavior is correct.
- `M3-02` Land instruction chain (CLASS/INHERIT/METHOD/GET_PROPERTY/SET_PROPERTY/GET_SUPER/INVOKE/SUPER_INVOKE)
  - Status: todo
  - Done criteria: class/inherit/this/super execute natively on VM.
- `M3-03` Solidify `init` constructor semantics in VM
  - Status: todo
  - Done criteria: constructor call returns receiver semantics reliably.
- `M3-04` Remove fallback usage (class cases)
  - Status: todo
  - Done criteria: class test family no longer triggers fallback.

### 11.8 M4 Progress Subsections (module VM protocol unification)

- `M4-01` Define and land module init context protocol (VM-only)
  - Status: todo
  - Done criteria: module top-level execution no longer depends on interpreter.
- `M4-02` Solidify export-binding order and visibility rules
  - Status: todo
  - Done criteria: partial-initialization scenarios have explicit behavior + tests.
- `M4-03` Align cache/failed state-machine behavior with spec
  - Status: todo
  - Done criteria: `MS5001~MS5004` behavior is stable and reproducible.
- `M4-04` Remove fallback usage (module cases)
  - Status: todo
  - Done criteria: module conformance is fallback-independent.

### 11.9 M5 Progress Subsections (real GC integration)

- `M5-01` Introduce Obj list and allocation registration
  - Status: todo
  - Done criteria: all objects are enumerable by GC framework.
- `M5-02` Connect root tracing (stack/frame/upvalue/globals/module)
  - Status: todo
  - Done criteria: root set is complete and observable.
- `M5-03` Land mark/blacken/sweep full cycle
  - Status: todo
  - Done criteria: unreachable objects are reclaimed without breaking live semantics.
- `M5-04` GC stress and stats closeout
  - Status: todo
  - Done criteria: stress tests pass; logs show reclaimed bytes and object-count trends.

### 11.10 M6 Progress Subsections (interpreter retirement and path convergence)

- `M6-01` Switch default execution mode to VM-only
  - Status: todo
  - Done criteria: normal execution path no longer calls `ScriptInterpreter`.
- `M6-02` Remove fallback-dependent assertions from tests
  - Status: todo
  - Done criteria: integration/conformance/diagnostics all treat VM as single path.
- `M6-03` Final interpreter disposition (delete or debug-only)
  - Status: todo
  - Done criteria: disposition strategy is implemented and documented consistently.
- `M6-04` Milestone final closeout
  - Status: todo
  - Done criteria: `fallback_rate == 0` (normal mode), full verification passes.

---

### 11.11 Progress Update Rules

1. Any milestone status change must update this section and `PLAN.md` on the same day.
2. Every `done` subsection must include at least one reproducible verification command/result summary.
3. Any `blocked` subsection must record blocker and unblock condition.
4. Prefer each commit to advance adjacent subsections within one milestone only.
5. Do not mark a milestone `done` before its closeout subsection (`M*-04`) is done.
