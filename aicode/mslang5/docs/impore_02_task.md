# Maple Task Breakdown for `impove_02` (Executable/Build/Test Atomic Mode)

Status: Proposal  
Date: 2026-03-15  
Source design: `docs/impove_02.md`

## 1. Purpose

This document splits `docs/impove_02.md` into atomic tasks.  
Each task must be independently executable with:

1. a concrete implementation scope,
2. a compile step,
3. C++ tests,
4. `.ms` script verification.

## 2. Status Legend

1. `todo`: not started
2. `doing`: in progress
3. `blocked`: blocked by dependency
4. `done`: completed with evidence

## 3. Shared Commands

Build commands:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Primary test commands:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_unit`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration`
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_migration_debt`
4. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`
5. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`

Script execution command template:

1. Windows: `build\\Debug\\maple_cli.exe <script.ms>`
2. Linux: `./build/maple_cli <script.ms>`

## 4. Task Tracking Table

| Task ID | Milestone | Atomic goal | Compile step | C++ tests | `.ms` scripts | Status | Owner | Evidence |
|---|---|---|---|---|---|---|---|---|
| T00 | Baseline | Freeze baseline and verify current green state | Full build | unit + integration + migration_debt + conformance + diagnostics | `tests/scripts/migration/m0_baseline_guardrails.ms` | todo | - | - |
| T01 | VM-only | Remove legacy interpreter build switch from CMake | Full build | unit + migration_debt | `tests/scripts/migration/m6_vm_only_convergence.ms` | todo | - | - |
| T02 | VM-only | Remove legacy interpreter source files and runtime hooks | Full build | unit + integration + migration_debt | `tests/scripts/migration/m6_vm_only_convergence.ms` | todo | - | - |
| T03 | VM-only | Simplify source mode/route enums and update tests | Full build | unit + integration + migration_debt | `tests/scripts/migration/m6_vm_only_convergence.ms` | todo | - | - |
| T04 | GC | Introduce VM-owned object header and allocation list scaffolding | Full build | unit + integration | `tests/scripts/migration/m5_gc_stress_mix.ms` | todo | - | - |
| T05 | GC | Replace shared ownership paths for closure/class/instance core objects | Full build | integration + conformance | `tests/scripts/language/closure_capture.ms`, `tests/scripts/language/class_super_chain.ms` | todo | - | - |
| T06 | GC | Complete root tracing and raw sweep reclamation | Full build | integration + diagnostics | `tests/scripts/migration/m5_gc_stress_mix.ms`, `tests/scripts/module/import_cache_and_alias.ms` | todo | - | - |
| T07 | GC | Add GC stress assertions and observability checks | Full build | integration (`test_gc.cc`) + unit (`test_vm_compiler.cc`) | `tests/scripts/migration/m5_gc_stress_mix.ms` | todo | - | - |
| T08 | Stdlib | Add native callable infrastructure in VM | Full build | unit + integration | `tests/scripts/cli_ok.ms` | todo | - | - |
| T09 | Stdlib | Implement `std.io` and `std.math` modules | Full build | integration + conformance | new scripts under `tests/scripts/module/std/` | todo | - | - |
| T10 | Stdlib | Implement `std.str`, `std.time`, `std.debug` modules | Full build | integration + conformance + diagnostics | new scripts under `tests/scripts/module/std/` | todo | - | - |
| T11 | Newline | Add newline tokenization in lexer with compatibility mode | Full build | unit (`test_lexer.cc`) + integration | new scripts under `tests/scripts/migration/newline/` | todo | - | - |
| T12 | Newline | Add parser/compiler statement-end abstraction (`consume_statement_end`) | Full build | unit (`test_vm_compiler.cc`) + integration + conformance | new scripts under `tests/scripts/migration/newline/` | todo | - | - |
| T13 | Newline | Remove semicolon dependency in grammar and finalize migration | Full build | conformance + diagnostics + integration | convert representative scripts to newline style | todo | - | - |
| T14 | Closeout | Run full regression and update design/spec docs | Full build | all suites | `tests/scripts/migration/m6_vm_only_convergence.ms` + newline/std scripts | todo | - | - |

## 5. Task Details (Atomic Execution Contract)

## T00 Baseline Freeze

1. Goal: capture current baseline behavior before structural refactor.
2. Code scope: no feature change; add/update tracking docs only if needed.
3. Compile: run shared build commands.
4. C++ tests: run all five ctest commands.
5. Script checks:
   - `tests/scripts/migration/m0_baseline_guardrails.ms`
6. Done criteria:
   - all suites pass,
   - baseline evidence logged in this file.

## T01 Remove Legacy CMake Switch

1. Goal: remove `MAPLE_ENABLE_LEGACY_INTERPRETER` from CMake options/definitions.
2. Code scope:
   - `CMakeLists.txt`
3. Compile: run shared build commands.
4. C++ tests:
   - `maple_tests_unit`
   - `maple_tests_migration_debt`
5. Script checks:
   - `tests/scripts/migration/m6_vm_only_convergence.ms`
6. Done criteria:
   - build contains no legacy compile option,
   - VM-only tests stay green.

## T02 Remove Legacy Runtime Hooks

1. Goal: remove interpreter fallback branches and old source files.
2. Code scope:
   - `src/runtime/vm.hh`
   - `src/runtime/vm.cc`
   - `src/runtime/script_interpreter.hh` (delete)
   - `src/runtime/script_interpreter.cc` (delete)
3. Compile: run shared build commands.
4. C++ tests:
   - unit + integration + migration_debt
5. Script checks:
   - `tests/scripts/migration/m6_vm_only_convergence.ms`
6. Done criteria:
   - no runtime path can invoke interpreter,
   - migration debt suite keeps `fallback_rate=0`.

## T03 Simplify Source Mode/Route Contracts

1. Goal: simplify enums/APIs that still imply dual-engine execution.
2. Code scope:
   - `src/runtime/vm.hh`
   - `src/runtime/vm.cc`
   - related tests in `tests/unit` and `tests/integration`
3. Compile: run shared build commands.
4. C++ tests:
   - unit + integration + migration_debt
5. Script checks:
   - `tests/scripts/migration/m6_vm_only_convergence.ms`
6. Done criteria:
   - execution contract documents only VM route.

## T04 GC Object Header + Allocation List Scaffold

1. Goal: establish VM-owned object metadata and allocation chain.
2. Code scope:
   - `src/runtime/object.hh`
   - `src/runtime/object.cc`
   - `src/runtime/gc.hh`
   - `src/runtime/gc.cc`
   - `src/runtime/vm.hh`
   - `src/runtime/vm.cc`
3. Compile: run shared build commands.
4. C++ tests:
   - unit + integration
5. Script checks:
   - `tests/scripts/migration/m5_gc_stress_mix.ms`
6. Done criteria:
   - object registration no longer depends on pointer-map-only semantics.

## T05 Replace Shared Ownership Paths in Runtime Objects

1. Goal: move core runtime object ownership away from `shared_ptr` graph semantics.
2. Code scope:
   - object/value/vm core files,
   - callframe/upvalue/object graph references.
3. Compile: run shared build commands.
4. C++ tests:
   - integration + conformance
5. Script checks:
   - `tests/scripts/language/closure_capture.ms`
   - `tests/scripts/language/class_super_chain.ms`
6. Done criteria:
   - closure/class semantics remain stable under VM-owned heap objects.

## T06 Complete Root Tracing + Raw Sweep

1. Goal: complete tracing roots and direct reclaim path.
2. Code scope:
   - GC tracer and sweep implementation,
   - VM/module/global root traversal.
3. Compile: run shared build commands.
4. C++ tests:
   - integration + diagnostics
5. Script checks:
   - `tests/scripts/migration/m5_gc_stress_mix.ms`
   - `tests/scripts/module/import_cache_and_alias.ms`
6. Done criteria:
   - no reachable object reclaimed,
   - unreachable objects reclaimed deterministically.

## T07 GC Stress + Metrics Closeout

1. Goal: harden GC behavior with stress validation and measurable stats.
2. Code scope:
   - `tests/integration/test_gc.cc`
   - `tests/unit/test_vm_compiler.cc`
   - optional GC debug output path.
3. Compile: run shared build commands.
4. C++ tests:
   - integration (GC focused) + unit
5. Script checks:
   - `tests/scripts/migration/m5_gc_stress_mix.ms`
6. Done criteria:
   - stable GC metrics across repeated test runs.

## T08 Native Callable Infrastructure

1. Goal: support VM-level native callables.
2. Code scope:
   - runtime object model,
   - call dispatch in VM,
   - stdlib registration entry.
3. Compile: run shared build commands.
4. C++ tests:
   - unit + integration
5. Script checks:
   - `tests/scripts/cli_ok.ms`
6. Done criteria:
   - native functions callable with arity/type diagnostics.

## T09 `std.io` + `std.math` Modules

1. Goal: land first standard-library subset.
2. Code scope:
   - std module bootstrap implementation,
   - module resolver hook for `std.*`.
3. Compile: run shared build commands.
4. C++ tests:
   - integration + conformance
5. Script checks:
   - new scripts in `tests/scripts/module/std/` covering io/math APIs.
6. Done criteria:
   - `import std.io` and `import std.math` pass API and error tests.

## T10 `std.str` + `std.time` + `std.debug` Modules

1. Goal: complete v1 standard library surface.
2. Code scope:
   - std module implementations and diagnostics.
3. Compile: run shared build commands.
4. C++ tests:
   - integration + conformance + diagnostics
5. Script checks:
   - new scripts in `tests/scripts/module/std/` covering str/time/debug APIs.
6. Done criteria:
   - documented APIs are stable and tested.

## T11 Newline Lexer Support

1. Goal: emit newline-sensitive tokens with compatibility behavior.
2. Code scope:
   - `src/frontend/token.hh`
   - `src/frontend/lexer.hh`
   - `src/frontend/lexer.cc`
   - lexer tests
3. Compile: run shared build commands.
4. C++ tests:
   - unit (`test_lexer.cc`) + integration
5. Script checks:
   - new scripts in `tests/scripts/migration/newline/`
6. Done criteria:
   - lexer output is stable for newline, braces, and EOF boundaries.

## T12 Parser/Compiler Statement-End Abstraction

1. Goal: replace direct semicolon consumption with unified statement-end handling.
2. Code scope:
   - `src/frontend/parser.hh`
   - `src/frontend/parser.cc`
   - `src/frontend/compiler.cc`
3. Compile: run shared build commands.
4. C++ tests:
   - unit (`test_vm_compiler.cc`) + integration + conformance
5. Script checks:
   - new scripts in `tests/scripts/migration/newline/`
6. Done criteria:
   - newline-terminated statements work with compatibility mode enabled.

## T13 Final Semicolon Retirement

1. Goal: remove semicolon dependency from grammar and implementation.
2. Code scope:
   - grammar/spec docs,
   - frontend token/parser/compiler,
   - script fixtures migrated to newline style.
3. Compile: run shared build commands.
4. C++ tests:
   - conformance + diagnostics + integration
5. Script checks:
   - converted representative scripts in `tests/scripts/language/`, `tests/scripts/module/`, and `tests/scripts/migration/newline/`
6. Done criteria:
   - semicolon is no longer required by language grammar.

## T14 Final Regression and Documentation Closeout

1. Goal: full-system closeout and evidence collection.
2. Code scope:
   - update `docs/impove_02.md`
   - update this task file status/evidence
   - update spec docs if needed.
3. Compile: run shared build commands.
4. C++ tests:
   - all ctest suites
5. Script checks:
   - `tests/scripts/migration/m6_vm_only_convergence.ms`
   - std/newline script sets
6. Done criteria:
   - all suites pass,
   - all task rows have final status and evidence.

## 6. Progress Update Rules

1. Only one task may be `doing` at a time.
2. Any task marked `done` must include command output summary in `Evidence`.
3. If a task is `blocked`, record blocker and unblock condition in `Evidence`.
4. Do not mark T14 `done` before all prior tasks are `done`.