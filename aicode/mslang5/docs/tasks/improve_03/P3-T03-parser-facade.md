# P3-T03 Parser Facade Extraction

Milestone: M2  
Stream: B

## Goal

Extract a parser-only boundary facade so parsing and bytecode emission are no longer interleaved in one control path.

## Implementation Scope

1. Introduce parser boundary interfaces in `src/frontend/`.
2. Keep existing compile output unchanged.
3. Add regression checks for parser output stability.

## Execution Steps

1. Add parser AST or parser intermediate structures required by the new boundary.
2. Route existing compile flow through parser facade.
3. Keep lowering path delegated to existing logic for parity.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_unit`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration`
3. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`

## Done Criteria

1. Parser boundary compiles and is used by main compile flow.
2. Existing semantic tests remain green.
3. No diagnostic code drift for unchanged cases.

## Dependencies

`P3-T02`.
