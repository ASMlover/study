# P3-T05 Lowering Phase Parity

Milestone: M2  
Stream: B

## Goal

Introduce a dedicated lowering phase from resolved frontend representation to bytecode, with parity checks against baseline output.

## Implementation Scope

1. Add lowering phase module(s) under `src/frontend/`.
2. Keep VM bytecode contract stable.
3. Add parity assertions in compiler-oriented tests.

## Execution Steps

1. Move bytecode emission out of parser/resolver flow.
2. Feed lowering only resolved metadata and syntax representation.
3. Add fixture comparisons for representative scripts.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_unit`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration`
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`

## Done Criteria

1. Lowering phase is isolated and deterministic.
2. Bytecode parity holds for selected baseline fixtures.
3. Verification commands pass.

## Dependencies

`P3-T04`.
