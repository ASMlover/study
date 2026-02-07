# Task 03 - Resolver Scope Binding

## Objective
Add static scope resolution and variable binding checks before interpretation.

## Scope
### In Scope
1. Scope stack management.
2. Variable declaration/definition lifecycle checks.
3. Binding depth output for interpreter lookup optimization.

### Out Of Scope
1. Full runtime execution engine changes.
2. Class and module resolver rules.

## Code Changes
1. `Resolver.hh`
2. `Resolver.cc`
3. `Interpreter.hh` (binding hook only if needed)
4. `tests/ResolverTest.cc`
5. `tests/scripts/resolver/scope_shadow.ms`
6. `tests/scripts/resolver/scope_self_init_error.ms`
7. `tests/expected/resolver/scope_shadow.out`
8. `tests/expected/resolver/scope_self_init_error.out`

## Implementation Steps
1. Implement resolver with begin/end scope stack API.
2. Track declared vs defined states and enforce no self-read during initializer.
3. Record expression-to-depth bindings for local lookup.
4. Emit standardized diagnostics on scope errors.
5. Wire resolver tests and optional script assertions.

## Testcases
### Unit/Component (C++)
1. Shadowing in nested scope resolves to nearest declaration.
2. Undefined variable usage reports error.
3. Self-reference in initializer reports error.

### E2E Script
1. Scope scripts produce expected resolve diagnostics.

### Negative/Error
1. Duplicate declaration in same scope triggers resolver error.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleResolverTest
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptSuite
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. Resolver validates scope rules and surfaces diagnostics with locations.
2. Resolver tests and full suite pass.

## Post-Completion Update
1. Update `Todos.md` row 03: `Status=done`, set `Done Date`.
2. Log resolver diagnostics coverage and executed test commands.
3. Advance row 04 only when all gates are green.

## Rollback Notes
1. If resolver integration breaks parser-only flow, keep resolver as optional phase behind explicit call until fixed.
