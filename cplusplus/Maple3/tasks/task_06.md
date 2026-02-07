# Task 06 - Functions, Calls, Returns, Closures

## Objective
Implement function declarations, invocation, return semantics, and lexical closures.

## Scope
### In Scope
1. Parse and represent function declarations and call expressions.
2. Runtime callable objects and call stack handling.
3. Closure capture and return propagation.

### Out Of Scope
1. Class/object model.
2. Module import.

## Code Changes
1. `Ast.hh`
2. `Ast.cc`
3. `Parser.hh`
4. `Parser.cc`
5. `Interpreter.hh`
6. `Interpreter.cc`
7. `Resolver.hh`
8. `Resolver.cc`
9. `tests/InterpreterFunctionTest.cc`
10. `tests/scripts/function/fn_basic.ms`
11. `tests/scripts/function/fn_recursion.ms`
12. `tests/scripts/function/fn_closure.ms`
13. `tests/scripts/function/fn_return_error.ms`
14. `tests/expected/function/fn_basic.out`
15. `tests/expected/function/fn_recursion.out`
16. `tests/expected/function/fn_closure.out`
17. `tests/expected/function/fn_return_error.out`

## Implementation Steps
1. Add AST nodes for function declarations, calls, and return statements.
2. Implement parser support for function declaration syntax and return statements.
3. Implement callable interface and function value object.
4. Implement closure environment capture at function declaration time.
5. Add resolver rules for function scope and invalid top-level return.

## Testcases
### Unit/Component (C++)
1. Function invocation with fixed arity.
2. Recursive function correctness.
3. Closure captures outer variable by lexical scope.

### E2E Script
1. Basic function and recursion scripts produce expected outputs.
2. Closure script validates captured-state behavior.

### Negative/Error
1. Arity mismatch diagnostic.
2. Illegal `return` context diagnostic.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleInterpreterFunctionTest
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptSuite
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. Functions, recursion, and closures are stable and tested.
2. Resolver and runtime errors are standardized and location-aware.

## Post-Completion Update
1. Update `Todos.md` row 06: `Status=done`, set `Done Date`.
2. Append test and script evidence to progress log.
3. Set row 07 to `in_progress` after complete green run.

## Rollback Notes
1. If closures are unstable, keep function call support and gate closure cases behind explicit TODO with failing tests fixed before merge.
