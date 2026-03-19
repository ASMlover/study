# Maple Task Breakdown for `impove_02` (Executable/Build/Test Atomic Mode)

Status: Proposal
Date: 2026-03-15
Source design: `docs/impove_02.md`

## 1. Purpose

This document splits `docs/impove_02.md` into atomic tasks. Each task must be independently:

1. implementable,
2. buildable,
3. verifiable by C++ tests,
4. verifiable by `.ms` scripts.

## 2. Status Legend

1. `todo`: not started
2. `doing`: in progress
3. `blocked`: blocked by dependency
4. `done`: completed with evidence

## 3. Shared Commands

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

CTest suites:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_unit`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration`
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_migration_debt`
4. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`
5. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`

Script execution template:

1. Windows: `build\\Debug\\maple_cli.exe <script.ms>`
2. Linux: `./build/maple_cli <script.ms>`

## 4. Task Tracking Table

| Task ID | Milestone | Atomic goal | Compile step | C++ tests | `.ms` scripts | Status | Owner |
|---|---|---|---|---|---|---|---|
| T00 | Baseline | Freeze baseline and verify current green state | Full build | unit + integration + migration_debt + conformance + diagnostics | `tests/scripts/migration/m0_baseline_guardrails.ms` | done | Codex |
| T01 | VM-only | Remove legacy interpreter build switch from CMake | Full build | unit + migration_debt | `tests/scripts/migration/m6_vm_only_convergence.ms` | done | Codex |
| T02 | VM-only | Remove legacy interpreter source files and runtime hooks | Full build | unit + integration + migration_debt | `tests/scripts/migration/m6_vm_only_convergence.ms` | done | Codex |
| T03 | VM-only | Simplify source mode/route enums and update tests | Full build | unit + integration + migration_debt | `tests/scripts/migration/m6_vm_only_convergence.ms` | done | Codex |
| T04 | GC | Introduce VM-owned object header and allocation list scaffolding | Full build | unit + integration | `tests/scripts/migration/m5_gc_stress_mix.ms` | done | Codex |
| T05 | GC | Replace shared ownership paths for closure/class/instance core objects | Full build | integration + conformance | `tests/scripts/language/closure_capture.ms`, `tests/scripts/language/class_super_chain.ms` | done | Codex |
| T06 | GC | Complete root tracing and raw sweep reclamation | Full build | integration + diagnostics | `tests/scripts/migration/m5_gc_stress_mix.ms`, `tests/scripts/module/import_cache_and_alias.ms` | done | Codex |
| T07 | GC | Add GC stress assertions and observability checks | Full build | integration (`test_gc.cc`) + unit (`test_vm_compiler.cc`) | `tests/scripts/migration/m5_gc_stress_mix.ms` | done | Codex |
| T08 | Stdlib | Add native callable infrastructure in VM | Full build | unit + integration | `tests/scripts/cli_ok.ms` | done | Codex |
| T09 | Stdlib | Implement `std.io` and `std.math` modules | Full build | integration + conformance | new scripts under `tests/scripts/module/std/` | done | Codex |
| T10 | Stdlib | Implement `std.str`, `std.time`, `std.debug` modules | Full build | integration + conformance + diagnostics | `str_time_debug_ok.ms`, `str_type_error.ms`, `time_arity_error.ms`, `debug_arity_error.ms` | done | Codex |
| T11 | Newline | Add newline tokenization in lexer with compatibility mode | Full build | unit (`test_lexer.cc`) + integration | new scripts under `tests/scripts/migration/newline/` | todo | - |
| T12 | Newline | Add parser/compiler statement-end abstraction (`consume_statement_end`) | Full build | unit (`test_vm_compiler.cc`) + integration + conformance | new scripts under `tests/scripts/migration/newline/` | todo | - |
| T13 | Newline | Remove semicolon dependency in grammar and finalize migration | Full build | conformance + diagnostics + integration | convert representative scripts to newline style | todo | - |
| T14 | Closeout | Run full regression and update design/spec docs | Full build | all suites | `tests/scripts/migration/m6_vm_only_convergence.ms` + newline/std scripts | todo | - |
## 5. Task Details (Atomic Execution Contract)

Per-task fixed template:

1. Goal
2. Code scope
3. Compile
4. C++ tests
5. Script checks
6. Done criteria

## T00 Baseline Freeze

1. Goal: capture baseline before structural refactor.
2. Code scope: no feature change; tracking docs only if required.
3. Compile: shared build commands.
4. C++ tests: all five ctest suites.
5. Script checks: `tests/scripts/migration/m0_baseline_guardrails.ms`.
6. Done criteria: all suites pass and baseline evidence is recorded.

## T01 Remove Legacy CMake Switch

1. Goal: remove `MAPLE_ENABLE_LEGACY_INTERPRETER` options/definitions.
2. Code scope: `CMakeLists.txt`.
3. Compile: shared build commands.
4. C++ tests: `maple_tests_unit`, `maple_tests_migration_debt`.
5. Script checks: `tests/scripts/migration/m6_vm_only_convergence.ms`.
6. Done criteria: no legacy compile option; VM-only tests remain green.

## T02 Remove Legacy Runtime Hooks

1. Goal: remove interpreter fallback branches and source files.
2. Code scope: `src/runtime/vm.hh`, `src/runtime/vm.cc`, delete `src/runtime/script_interpreter.hh/.cc`.
3. Compile: shared build commands.
4. C++ tests: unit + integration + migration_debt.
5. Script checks: `tests/scripts/migration/m6_vm_only_convergence.ms`.
6. Done criteria: runtime cannot invoke interpreter; migration debt keeps `fallback_rate=0`.

## T03 Simplify Source Mode/Route Contracts

1. Goal: remove dual-engine implications from mode/route APIs.
2. Code scope: `src/runtime/vm.hh`, `src/runtime/vm.cc`, related tests.
3. Compile: shared build commands.
4. C++ tests: unit + integration + migration_debt.
5. Script checks: `tests/scripts/migration/m6_vm_only_convergence.ms`.
6. Done criteria: execution contract is VM-only.

## T04 GC Object Header + Allocation Scaffold

1. Goal: add VM-owned object metadata and allocation chain.
2. Code scope: `src/runtime/object.hh/.cc`, `src/runtime/gc.hh/.cc`, `src/runtime/vm.hh/.cc`.
3. Compile: shared build commands.
4. C++ tests: unit + integration.
5. Script checks: `tests/scripts/migration/m5_gc_stress_mix.ms`.
6. Done criteria: GC registration is not pointer-map-only.

## T05 Replace Shared Ownership Paths in Core Objects

1. Goal: remove `shared_ptr`-centric ownership from core runtime object graph.
2. Code scope: object/value/vm core files and frame/upvalue/object references.
3. Compile: shared build commands.
4. C++ tests: integration + conformance.
5. Script checks: `tests/scripts/language/closure_capture.ms`, `tests/scripts/language/class_super_chain.ms`.
6. Done criteria: closure/class semantics remain stable under VM-owned heap semantics.

## T06 Complete Root Tracing + Raw Sweep

1. Goal: complete root coverage and direct sweep reclamation.
2. Code scope: GC tracer/sweeper and VM/module/global root traversal.
3. Compile: shared build commands.
4. C++ tests: integration + diagnostics.
5. Script checks: `tests/scripts/migration/m5_gc_stress_mix.ms`, `tests/scripts/module/import_cache_and_alias.ms`.
6. Done criteria: reachable objects are preserved and unreachable objects are reclaimed deterministically.

## T07 GC Stress + Metrics Closeout

1. Goal: stabilize GC under stress with measurable metrics.
2. Code scope: `tests/integration/test_gc.cc`, `tests/unit/test_vm_compiler.cc`, optional GC debug output.
3. Compile: shared build commands.
4. C++ tests: integration (GC focused) + unit.
5. Script checks: `tests/scripts/migration/m5_gc_stress_mix.ms`.
6. Done criteria: GC metrics are stable across repeated runs.

## T08 Native Callable Infrastructure

1. Goal: support VM-native callable builtins.
2. Code scope: runtime object model, VM call dispatch, stdlib registration entry.
3. Compile: shared build commands.
4. C++ tests: unit + integration.
5. Script checks: `tests/scripts/cli_ok.ms`.
6. Done criteria: native functions are callable with correct arity/type diagnostics.

## T09 `std.io` + `std.math`

1. Goal: implement first stdlib subset.
2. Code scope: std bootstrap and `std.*` resolver hooks.
3. Compile: shared build commands.
4. C++ tests: integration + conformance.
5. Script checks: new scripts in `tests/scripts/module/std/` for io/math API and errors.
6. Done criteria: `import std.io` and `import std.math` pass functional/error tests.

## T10 `std.str` + `std.time` + `std.debug`

1. Goal: complete stdlib v1 surface.
2. Code scope: std module implementations and diagnostics.
3. Compile: shared build commands.
4. C++ tests: integration + conformance + diagnostics.
5. Script checks: new scripts in `tests/scripts/module/std/` for str/time/debug API and errors.
6. Done criteria: documented APIs are stable and tested.

## T11 Newline Lexer Support

1. Goal: emit newline-sensitive tokens with compatibility mode.
2. Code scope: `src/frontend/token.hh`, `src/frontend/lexer.hh`, `src/frontend/lexer.cc`, lexer tests.
3. Compile: shared build commands.
4. C++ tests: unit (`test_lexer.cc`) + integration.
5. Script checks: new scripts in `tests/scripts/migration/newline/`.
6. Done criteria: newline/braces/EOF boundaries tokenize stably.

## T12 Parser/Compiler Statement-End Abstraction

1. Goal: replace semicolon-only handling with unified statement-end handling.
2. Code scope: `src/frontend/parser.hh`, `src/frontend/parser.cc`, `src/frontend/compiler.cc`.
3. Compile: shared build commands.
4. C++ tests: unit (`test_vm_compiler.cc`) + integration + conformance.
5. Script checks: new scripts in `tests/scripts/migration/newline/`.
6. Done criteria: newline-terminated statements work in compatibility mode.

## T13 Final Semicolon Retirement

1. Goal: remove semicolon dependency from grammar and implementation.
2. Code scope: spec grammar, frontend token/parser/compiler, migrated script fixtures.
3. Compile: shared build commands.
4. C++ tests: conformance + diagnostics + integration.
5. Script checks: converted scripts in `tests/scripts/language/`, `tests/scripts/module/`, `tests/scripts/migration/newline/`.
6. Done criteria: semicolon is no longer required by language grammar.

## T14 Final Regression and Documentation Closeout

1. Goal: complete system closeout and evidence archive.
2. Code scope: update `docs/impove_02.md`, this task file, and related spec docs.
3. Compile: shared build commands.
4. C++ tests: all suites.
5. Script checks: `tests/scripts/migration/m6_vm_only_convergence.ms` + std/newline script sets.
6. Done criteria: all suites pass and all task rows contain final status/evidence.

## 6. Progress Update Rules

1. Only one task can be `doing` at a time.
2. Any `done` task must include reproducible evidence in the table.
3. Any `blocked` task must include blocker + unblock condition.
4. T14 cannot be `done` before all preceding tasks are `done`.

