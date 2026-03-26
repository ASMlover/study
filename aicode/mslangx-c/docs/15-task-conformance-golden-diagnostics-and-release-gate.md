# Task 15 - Conformance, Golden Diagnostics, Release Gate

## Goal

Turn the earlier task work into a stable verification and release baseline so future changes do not silently break the language contract.

## Design Links

- Test baseline and diagnostic families: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 01 through Task 14

## Scope

1. Define the test taxonomy and directory conventions.
2. Add conformance coverage for the language surface.
3. Add golden diagnostics that lock `phase + code + line`.
4. Add end-to-end regression coverage for modules and GC stress.
5. Define the local and CI release gate.

## Implementation Boundaries

1. This task owns test governance and release criteria, not new runtime features.
2. Golden updates must be explicit and justified rather than incidental.
3. The final gate must use one entrypoint that developers can run locally.
4. Task 15 should be updated incrementally as earlier tasks land, then finalized after Task 14.

## File Ownership

1. `tests/conformance/`
2. `tests/golden/`
3. `tests/integration/`
4. `tests/stress/`
5. one unified test runner script under `tests/` or repository root
6. one testing/release-gate doc if extra process documentation is needed

## Release Gate

1. Configure and Debug build succeed.
2. Unit, integration, conformance, golden, and stress suites all pass.
3. `.ms` coverage exists for `fn`, `self`, `super`, `break`, `continue`,
   evaluation order, short-circuiting, containers, and modules.
4. Diagnostics lock `phase + code + line`.
5. GC stress and module regressions are part of the standard gate.

## TDD and Process Policy

1. Every new language feature must add at least one `.ms` conformance case.
2. Any feature that introduces new failures must add negative diagnostic coverage.
3. Golden updates require an explicit rationale in the change description.
4. Conformance coverage must include module-namespace isolation and illegal
   `return <expr>` in `init`.
5. No task is done while the full gate is red.

## Acceptance

1. One documented command runs the full matrix locally.
2. Local execution and CI use equivalent test scopes.
3. Conformance and golden assets are organized so failures are easy to triage.
4. The task is not complete until build passes, the full matrix passes, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Out of Scope

1. New language syntax.
2. New runtime capabilities.
