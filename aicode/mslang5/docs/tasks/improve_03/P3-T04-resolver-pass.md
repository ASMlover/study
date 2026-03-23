# P3-T04 Resolver Metadata Pass

Milestone: M2  
Stream: B

## Goal

Introduce a resolver pass that computes lexical scope metadata independent of bytecode emission.

## Implementation Scope

1. Add resolver entry points in `src/frontend/`.
2. Persist resolver metadata required by lowering.
3. Add scope/inheritance/closure resolver tests.

## Execution Steps

1. Move scope-depth and capture bookkeeping into resolver pass.
2. Keep emitted bytecode equivalent to baseline.
3. Add resolver-focused negative tests for invalid symbol usage.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_unit`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`

## Done Criteria

1. Resolver pass is explicit and reusable.
2. Closure/class/module scope behavior remains stable.
3. Verification commands pass.

## Dependencies

`P3-T03`.
