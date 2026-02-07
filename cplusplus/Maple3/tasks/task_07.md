# Task 07 - Class, Instance, this, super

## Objective
Add object-oriented features: classes, instances, inheritance, method binding, `this`, and `super`.

## Scope
### In Scope
1. Parse and represent class declarations and property access.
2. Runtime class and instance objects with method dispatch.
3. Resolver checks for `this` and `super` legality.

### Out Of Scope
1. Module import feature.
2. CLI command UX improvements.

## Code Changes
1. `Ast.hh`
2. `Ast.cc`
3. `Parser.hh`
4. `Parser.cc`
5. `Interpreter.hh`
6. `Interpreter.cc`
7. `Resolver.hh`
8. `Resolver.cc`
9. `tests/InterpreterClassTest.cc`
10. `tests/scripts/class/class_basic.ms`
11. `tests/scripts/class/class_inherit.ms`
12. `tests/scripts/class/class_super.ms`
13. `tests/scripts/class/class_this_error.ms`
14. `tests/expected/class/class_basic.out`
15. `tests/expected/class/class_inherit.out`
16. `tests/expected/class/class_super.out`
17. `tests/expected/class/class_this_error.out`

## Implementation Steps
1. Add AST nodes for class declarations, property get/set, `this`, `super`.
2. Extend parser for class syntax and method bodies.
3. Implement runtime class and instance objects with field storage.
4. Implement method lookup and bound-method `this` context.
5. Add inheritance and `super` method resolution.

## Testcases
### Unit/Component (C++)
1. Method binding keeps correct `this`.
2. Field set/get on instances.
3. Inheritance chain method lookup.

### E2E Script
1. Class construction and method calls output expected lines.
2. Inheritance override + `super` call behavior.

### Negative/Error
1. Use of `this` outside class reports resolver error.
2. Use of `super` without superclass reports resolver error.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleInterpreterClassTest
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptSuite
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. OOP scripts and component tests are green.
2. Diagnostics for invalid `this/super` are clear and located.

## Post-Completion Update
1. Update `Todos.md` row 07: `Status=done`, set `Done Date`.
2. Add progress summary and tested script names.
3. Move row 08 to `in_progress` only after full green suite.

## Rollback Notes
1. If inheritance path is unstable, keep base class functionality while fixing `super` resolution before closing task.
