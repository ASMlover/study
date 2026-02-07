# Task 04 - Runtime Value And Environment

## Objective
Build runtime primitives: value representation and lexical environment chain.

## Scope
### In Scope
1. Value variant type and core helpers.
2. Environment define/get/assign with parent chain.
3. Runtime component tests.

### Out Of Scope
1. Full interpreter statement execution.
2. Function/class/module semantics.

## Code Changes
1. `Value.hh`
2. `Value.cc`
3. `Environment.hh`
4. `Environment.cc`
5. `tests/ValueTest.cc`
6. `tests/EnvironmentTest.cc`

## Implementation Steps
1. Implement `Value` type covering `number/string/bool/nil` and placeholders for callable/object kinds.
2. Implement equality, truthiness, and debug string conversion.
3. Implement `Environment` with optional enclosing scope and ancestor lookup.
4. Add strict errors for undefined variable get/assign.
5. Register and run component tests.

## Testcases
### Unit/Component (C++)
1. Value equality and type-sensitive comparisons.
2. Truthiness semantics for nil/bool/non-bool.
3. Environment define/get/assign across nested scopes.

### E2E Script
1. Minimal script that exercises variable set/get path once interpreter hooks exist.

### Negative/Error
1. Assign or read undefined variable returns runtime diagnostic.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleValueTest
ctest --test-dir build --output-on-failure -C Debug -R MapleEnvironmentTest
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. Runtime primitives are usable by interpreter layer.
2. Value and environment tests are green.

## Post-Completion Update
1. Update `Todos.md` row 04: `Status=done`, set `Done Date`.
2. Append command/test summary to `Progress Log`.
3. Set row 05 to `in_progress` after green suite.

## Rollback Notes
1. If variant design causes broad compile breakage, first stabilize interfaces then incrementally add kinds.
