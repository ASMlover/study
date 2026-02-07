# Task 05 - Interpreter Core Execution

## Objective
Implement expression evaluation and core statement execution over runtime environment.

## Scope
### In Scope
1. Evaluate arithmetic/comparison/logical expressions.
2. Execute `print`, expression statement, variable declaration, block, `if`, `while`, `for`.
3. Integrate parser + resolver + interpreter main flow for script execution.

### Out Of Scope
1. Functions and closures.
2. Class/object model.
3. Module import.

## Code Changes
1. `Interpreter.hh`
2. `Interpreter.cc`
3. `Main.cc` (temporary run path if needed)
4. `tests/InterpreterCoreTest.cc`
5. `tests/scripts/interpreter/runtime_arith.ms`
6. `tests/scripts/interpreter/runtime_control_flow.ms`
7. `tests/scripts/interpreter/runtime_logic.ms`
8. `tests/expected/interpreter/runtime_arith.out`
9. `tests/expected/interpreter/runtime_control_flow.out`
10. `tests/expected/interpreter/runtime_logic.out`

## Implementation Steps
1. Add visitor/evaluation dispatch for expression AST.
2. Add execution dispatch for statement AST.
3. Implement lexical scope entering/exiting for blocks.
4. Ensure runtime and parse errors use unified diagnostic format.
5. Add interpreter core tests and script regression cases.

## Testcases
### Unit/Component (C++)
1. Binary/unary evaluation correctness.
2. Logical short-circuit behavior.
3. Block scope variable visibility.

### E2E Script
1. Arithmetic result output.
2. If/while/for control flow output.
3. Logical expression output.

### Negative/Error
1. Invalid operand types produce runtime diagnostics with location.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleInterpreterCoreTest
ctest --test-dir build --output-on-failure -C Debug -R MapleScriptSuite
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. Core scripts execute correctly via interpreter path.
2. Component tests and script suite are green.

## Post-Completion Update
1. Update `Todos.md` row 05: `Status=done`, set `Done Date`.
2. Record script names and command outputs in progress log.
3. Set row 06 to `in_progress` only when all tests pass.

## Rollback Notes
1. If parser-interpreter integration fails, preserve parser output tests and isolate interpreter entry until fixed.
