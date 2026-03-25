# P3-T07 Runtime Span Precision

Milestone: M3  
Stream: C

## Goal

Plumb source spans into runtime and module error paths so line-level precision reaches at least 95% of runtime-reporting cases.

## Implementation Scope

1. Extend runtime error-reporting structures in `src/runtime/`.
2. Propagate call-site/module context through VM execution paths.
3. Add precision-focused diagnostics tests.

## Execution Steps

1. Add optional span payload to runtime diagnostic records.
2. Fill spans at throw sites for operator/type/arity/module failures.
3. Add coverage accounting for span-present vs span-missing paths.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_integration`

## Done Criteria

1. Runtime diagnostics include line information for >=95% tracked error paths.
2. Compatibility key (`phase + code`) remains unchanged.
3. Verification commands pass.

## Dependencies

`P3-T02`.
