# Task 01 - Build Bootstrap and Test Harness

## Status

Complete. Verified on 2026-03-28 with the acceptance commands in this document.

## Goal

Create the minimal project skeleton that can be configured, built, executed, and tested before any language feature work starts.

## Design Links

- Baseline architecture and phase ordering: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

None.

## Scope

1. Create the pure C project layout.
2. Add CMake configure/build support for Debug and Release.
3. Produce a minimal `mslangc` CLI binary.
4. Add a test target and `ctest` integration.
5. Add one smoke test that proves the binary starts and exits cleanly.

## Implementation Boundaries

1. This task owns build, test, and CLI bootstrap only.
2. Do not add runtime value types, lexer, parser, VM, or GC logic here.
3. `src/main.c` may parse `--help`, `-e`, and file-path arguments, but execution may stay stubbed until later tasks.
4. Freeze test target naming and `ctest` label conventions now to avoid later harness churn.

## File Ownership

1. `CMakeLists.txt`
2. optional `cmake/` helpers
3. `src/main.c`
4. `include/` bootstrap headers as needed
5. `tests/` harness bootstrap and first smoke test

## TDD Plan

1. Write the first failing smoke test for `mslangc --help` before implementing the CLI.
2. Add a failing configure/build check to the documented command sequence.
3. Implement the minimal binary and test target only after the test entrypoints exist.
4. Keep this task free of placeholder tests that do not execute the binary.
5. `.ms` script coverage is not required in this task.

## Acceptance

1. `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` succeeds.
2. `cmake --build build --config Debug` succeeds.
3. `mslangc --help` returns exit code `0`.
4. `ctest` runs at least one passing smoke test.
5. The task is not complete until build passes, tests pass, the binary runs end to end for the covered path, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Out of Scope

1. Any language feature implementation.
2. Frontend or VM internals.
3. GC or module support.