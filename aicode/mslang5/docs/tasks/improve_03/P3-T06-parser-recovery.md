# P3-T06 Parser Recovery Synchronization

Milestone: M2  
Stream: B

## Goal

Implement parser synchronization and bounded cascade recovery so single syntax errors do not produce excessive downstream noise.

## Implementation Scope

1. Update parser recovery logic in `src/frontend/parser*.{hh,cc}`.
2. Add malformed-input fixtures under `tests/scripts/diagnostics/`.
3. Add explicit assertions for maximum cascade count.

## Execution Steps

1. Define synchronization anchors (statement boundary, block boundary, EOF).
2. Cap per-file error cascade with deterministic limits.
3. Ensure first error location remains stable.

## Verification

Build:

1. `cmake -S . -B build`
2. `cmake --build build --config Debug`

Tests:

1. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_diagnostics`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_tests_conformance`

## Done Criteria

1. Recovery behavior is deterministic and bounded.
2. First failure position is preserved across runs.
3. Verification commands pass.

## Dependencies

`P3-T05`.
