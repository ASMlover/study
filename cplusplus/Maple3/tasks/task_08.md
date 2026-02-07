# Task 08 - Module Import System

## Objective
Implement module loading and `import` execution semantics with cache and cycle protection.

## Scope
### In Scope
1. Parser support for import statements.
2. Module loader for path resolution and cache.
3. Import runtime execution and export binding behavior.

### Out Of Scope
1. Package manager or remote module fetching.
2. Advanced module versioning.

## Code Changes
1. `ModuleLoader.hh`
2. `ModuleLoader.cc`
3. `Parser.hh`
4. `Parser.cc`
5. `Interpreter.hh`
6. `Interpreter.cc`
7. `tests/ModuleLoaderTest.cc`
8. `tests/scripts/module/module_basic.ms`
9. `tests/scripts/module/module_nested.ms`
10. `tests/scripts/module/module_cycle_error.ms`
11. `tests/expected/module/module_basic.out`
12. `tests/expected/module/module_nested.out`
13. `tests/expected/module/module_cycle_error.out`

## Implementation Steps
1. Define import syntax and AST node behavior.
2. Implement path resolution rules (relative to current script first).
3. Implement module cache to prevent duplicate execution.
4. Detect and report circular import chain.
5. Add tests for loader internals and script-level behavior.

## Testcases
### Unit/Component (C++)
1. Module path resolution correctness.
2. Cache hit avoids re-execution.
3. Circular dependency detection returns deterministic diagnostic.

### E2E Script
1. Basic import and nested import scripts produce expected output.

### Negative/Error
1. Missing module file and cycle module scenarios produce clear errors.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleModuleLoaderTest
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptSuite
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. Import semantics are functional and deterministic.
2. Module loader tests and script suite are fully green.

## Post-Completion Update
1. Update `Todos.md` row 08: `Status=done`, set `Done Date`.
2. Record module test coverage and command outcomes in progress log.
3. Move row 09 to `in_progress` after all checks pass.

## Rollback Notes
1. If import parser syntax causes conflicts, isolate feature with explicit grammar branch and keep old grammar stable.
