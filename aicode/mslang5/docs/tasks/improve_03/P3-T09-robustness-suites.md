# P3-T09 Robustness Suites and CI Policy

Milestone: M4  
Stream: D

## Goal

Add robustness verification layers (fuzz seeds, bytecode invariants, long-run GC stress) and connect them to CI tiers.

## Implementation Scope

1. Add test drivers/fixtures under `tests/`.
2. Add CI wiring in `CMakeLists.txt` or test orchestration scripts.
3. Add deterministic triage metadata for flaky/non-deterministic runs.

## Execution Steps

1. Introduce parser fuzz seed corpus and expected crash-free run checks.
2. Add bytecode invariant checks before VM execution.
3. Add multi-iteration GC stress workflow with trend assertions.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_unit`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration`
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_migration_debt`

## Done Criteria

1. Robustness suites are versioned and runnable in CI.
2. Deterministic vs non-deterministic cases are labeled.
3. Verification commands pass.

## Dependencies

`P3-T06`, `P3-T08`.
