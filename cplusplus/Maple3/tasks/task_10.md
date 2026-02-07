# Task 10 - Full Language Regression Pack

## Objective
Build a comprehensive and maintainable language regression suite for long-term stability.

## Scope
### In Scope
1. Add broad script coverage across all language features.
2. Organize expected outputs by feature area.
3. Document fixture conventions and extension workflow.

### Out Of Scope
1. New language features not yet implemented.
2. Performance benchmarking infrastructure.

## Code Changes
1. `tests/scripts/language/**/*.ms`
2. `tests/expected/language/**/*.out`
3. `tests/README.md`
4. `CMakeLists.txt` (if additional suite sharding is needed)

## Implementation Steps
1. Create feature folders: scanner, parser, resolver, runtime, function, class, module, cli.
2. For each feature add normal, boundary, and error scripts.
3. Add deterministic expected outputs and diagnostic snapshots.
4. Ensure script suite runner supports grouped execution summaries.
5. Document naming and fixture authoring guidelines.

## Testcases
### Unit/Component (C++)
1. Keep all prior component tests green.

### E2E Script
1. Add and pass at least 60 scripts total.
2. Verify each feature has normal, boundary, error coverage.

### Negative/Error
1. Every error script must assert exact diagnostic prefix (`file:line:column error:`).

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptSuite
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. Regression suite reaches target coverage and remains deterministic.
2. Full CTest suite is green.
3. `tests/README.md` clearly describes how to add new scripts.

## Post-Completion Update
1. Update `Todos.md` row 10: `Status=done`, set `Done Date`.
2. Append final project progress entry with script counts and full command results.
3. Confirm all rows 01..10 are `done`.

## Rollback Notes
1. If flaky scripts appear, isolate by feature folder and fix determinism before merging.
