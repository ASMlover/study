# Maple v0.3 Atomic Execution Task Split

Status: Active  
Date: 2026-03-24  
Source design: `docs/improve_03.md`

## 1. Purpose

This document decomposes `docs/improve_03.md` into atomic tasks.  
Each task is required to be:

1. Implementable in a single focused change set.
2. Executable with explicit build/run steps.
3. Verifiable by deterministic test evidence.

## 2. Status Legend

1. `todo`: not started.
2. `doing`: in progress.
3. `blocked`: waiting for prerequisite or decision.
4. `done`: completed with reproducible evidence.

## 3. Shared Commands

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Core CTest suites:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_unit`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration`
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`
4. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`
5. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_migration_debt`

Script execution template:

1. Windows: `build\Debug\maple_cli.exe <script.ms>`
2. Linux: `./build/maple_cli <script.ms>`

## 4. Progress Tracking

| Task ID | Stream | Milestone | Atomic Goal | Detail Doc | Status | Owner | Start Date | End Date | Evidence/Notes |
|---|---|---|---|---|---|---|---|---|---|
| P3-T01 | A | M1 | Add spec stability tier markers and compatibility notes | `docs/tasks/improve_03/P3-T01-spec-stability.md` | done | Codex | 2026-03-24 | 2026-03-24 | Updated `docs/spec/semantics.md`, `docs/spec/errors.md`, and `docs/spec/modules.md` with clause-tier compatibility notes and provisional mappings; verified via `cmake -S . -B build`, `cmake --build build --config Debug`, `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`, and `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`. |
| P3-T02 | A | M1 | Add diagnostics code normalization check and fixture coverage | `docs/tasks/improve_03/P3-T02-diagnostics-normalization.md` | todo | TBD | - | - | - |
| P3-T03 | B | M2 | Extract parser boundary facade with behavior parity | `docs/tasks/improve_03/P3-T03-parser-facade.md` | todo | TBD | - | - | - |
| P3-T04 | B | M2 | Introduce resolver metadata pass and scope invariants | `docs/tasks/improve_03/P3-T04-resolver-pass.md` | todo | TBD | - | - | - |
| P3-T05 | B | M2 | Introduce lowering phase with bytecode parity assertions | `docs/tasks/improve_03/P3-T05-lowering-parity.md` | todo | TBD | - | - | - |
| P3-T06 | B | M2 | Implement parser recovery synchronization and cascade bound checks | `docs/tasks/improve_03/P3-T06-parser-recovery.md` | todo | TBD | - | - | - |
| P3-T07 | C | M3 | Add runtime/module line-span precision plumbing | `docs/tasks/improve_03/P3-T07-runtime-span-precision.md` | todo | TBD | - | - | - |
| P3-T08 | C | M3 | Enforce VM ownership and GC root invariants with test assertions | `docs/tasks/improve_03/P3-T08-gc-invariants.md` | todo | TBD | - | - | - |
| P3-T09 | D | M4 | Add fuzz/invariant/stress robustness suites and CI policy | `docs/tasks/improve_03/P3-T09-robustness-suites.md` | todo | TBD | - | - | - |
| P3-T10 | D | M4 | Implement release gates and KPI trend reporting workflow | `docs/tasks/improve_03/P3-T10-release-gates-kpi.md` | todo | TBD | - | - | - |

## 5. Progress Update Rules

1. Only one task may be `doing` at any time.
2. A task can be set to `done` only after required verification commands are recorded in `Evidence/Notes`.
3. `blocked` tasks must include both blocker description and unblock condition.
4. `P3-T10` cannot be `done` before `P3-T01` through `P3-T09` are `done`.

