# P3-T02 Diagnostics Normalization Check

Milestone: M1  
Stream: A

## Goal

Create a deterministic mapping contract for diagnostics (`phase + code + normalized span`) and enforce it with golden-based checks.

## Implementation Scope

1. Update diagnostics contract docs under `docs/spec/diagnostics*.md`.
2. Add or update test fixtures under `tests/scripts/diagnostics/`.
3. Add/extend diagnostics tests under `tests/diagnostics/`.

## Execution Steps

1. Define normalization rules for line/column and message-independent matching.
2. Add fixture cases for parse, resolve, runtime, and module failures.
3. Assert normalized output in diagnostics tests.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`

## Done Criteria

1. Normalization rules are documented and test-backed.
2. Golden checks no longer depend on free-form message text.
3. Verification commands pass.

## Dependencies

`P3-T01`.
