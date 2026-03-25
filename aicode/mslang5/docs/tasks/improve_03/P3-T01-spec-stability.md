# P3-T01 Spec Stability Markers

Milestone: M1  
Stream: A

## Goal

Add explicit `stable` / `provisional` markers for v0.2 normative clauses and add compatibility notes for each provisional item.

## Implementation Scope

1. Update spec clauses in `docs/spec/semantics.md`, `docs/spec/errors.md`, and `docs/spec/modules.md`.
2. Add a compact compatibility note section in each touched file.
3. Keep existing language behavior unchanged.

## Execution Steps

1. Mark every normative clause with one stability tier label.
2. Add compatibility promises for all `stable` rules.
3. Add migration caveats for all `provisional` rules.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`

## Done Criteria

1. All normative clauses have stability tiers.
2. No behavior change is introduced.
3. Verification commands pass.

## Dependencies

None.
