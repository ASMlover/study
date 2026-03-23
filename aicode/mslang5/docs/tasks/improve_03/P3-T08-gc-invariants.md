# P3-T08 VM Ownership and GC Invariants

Milestone: M3  
Stream: C

## Goal

Enforce VM-owned heap invariants and GC root coverage with explicit assertions and regression tests.

## Implementation Scope

1. Add invariant checks in GC/runtime code paths under `src/runtime/`.
2. Add GC-focused tests under `tests/integration/`.
3. Keep allocation and sweep behavior deterministic in debug builds.

## Execution Steps

1. Assert root-set coverage for stack, call frames, upvalues, globals, and module cache.
2. Assert forbidden ownership states defined by ADR.
3. Add stress scripts for repeated allocate/collect cycles.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_migration_debt`
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`

## Done Criteria

1. Ownership and root invariants are executable checks, not documentation only.
2. GC stress scenarios run without leaks/crashes under repeated cycles.
3. Verification commands pass.

## Dependencies

`P3-T07`.
