# Task 01 - Testing And Script Regression Framework

## Objective
Create a stable dual-track testing foundation: C++ tests plus script regression tests executed by CTest.

## Scope
### In Scope
1. Add script suite runner target and CTest registration.
2. Add sample script fixtures and expected outputs.
3. Add component tests for script runner behavior.

### Out Of Scope
1. Parser statement expansion.
2. Runtime semantics changes.

## Code Changes
1. `CMakeLists.txt`
2. `tests/ScriptRunner.cc`
3. `tests/ScriptRunner.hh`
4. `tests/ScriptRunnerTest.cc`
5. `tests/scripts/smoke/hello.ms`
6. `tests/scripts/smoke/arith.ms`
7. `tests/expected/smoke/hello.out`
8. `tests/expected/smoke/arith.out`

## Implementation Steps
1. Implement a script suite runner that discovers `.ms` files and expected `.out` files.
2. Add deterministic output comparison (exact match, normalized line ending).
3. Print diff-like diagnostics on mismatch.
4. Register `MapleScriptRunnerTest` and `MapleScriptSuite` in CMake + CTest.
5. Ensure existing tests (`MapleScanTest`, `MapleParserTest`) remain green.

## Testcases
### Unit/Component (C++)
1. Discover script files from nested directories.
2. Missing expected output reports a clear failure.
3. Output mismatch reports script path and first mismatch position.

### E2E Script
1. `hello.ms` returns expected hello output.
2. `arith.ms` returns expected arithmetic output.

### Negative/Error
1. Broken fixture pair should fail `MapleScriptSuite` with non-zero exit.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptRunnerTest
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptSuite
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. `MapleScriptRunnerTest` and `MapleScriptSuite` are visible in CTest and passing.
2. Existing tests keep passing.
3. Script fixtures are deterministic and cross-platform line ending safe.

## Post-Completion Update
1. Update `Todos.md` row 01: set `Status=done`, set `Done Date`.
2. Append `Progress Log` entry with summary and command results.
3. Set row 02 to `in_progress` only if task 01 is fully green.

## Rollback Notes
1. If CTest registration breaks, revert only added script targets and keep existing tests intact.
2. Do not merge partial runner without at least two green smoke scripts.
