# Task 02 - Parser Statements And Declarations

## Objective
Expand parsing from expressions to statements and declarations: `var`, `print`, block, `if`, `while`, `for`.

## Scope
### In Scope
1. Extend AST statement node set.
2. Implement parser declaration and statement entry points.
3. Keep parse error recovery robust.

### Out Of Scope
1. Runtime execution of statements.
2. Resolver semantic checks.

## Code Changes
1. `Ast.hh`
2. `Ast.cc`
3. `Parser.hh`
4. `Parser.cc`
5. `tests/ParserStatementTest.cc`
6. `tests/scripts/parser/stmt_print.ms`
7. `tests/scripts/parser/stmt_if_while.ms`
8. `tests/expected/parser/stmt_print.out`
9. `tests/expected/parser/stmt_if_while.out`

## Implementation Steps
1. Add statement AST nodes with debug-string support.
2. Add parser methods: `declaration`, `statement`, `var_declaration`, `block`, `if_statement`, `while_statement`, `for_statement`.
3. Keep `parse_program()` as the top-level API that returns full statement list.
4. Keep synchronization at semicolon and statement boundary keywords.
5. Add test target and CTest registration.

## Testcases
### Unit/Component (C++)
1. Parse `var` declarations with and without initializer.
2. Parse nested block statements.
3. Parse `if/else` and `while`.
4. Parse `for` desugaring path if implemented.

### E2E Script
1. Parser-only script suite for statement forms produces expected AST debug output.

### Negative/Error
1. Missing semicolon and missing right parenthesis recover and continue parsing later statements.

## Run Commands
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure -C Debug -R MapleParserStatementTest
ctest --test-dir build --output-on-failure -C Debug -R MapleParserTest
ctest --test-dir build --output-on-failure -C Debug
```

## Definition Of Done
1. Parser can parse statement/declaration program forms.
2. Parse errors include location and do not stop all subsequent parsing.
3. Parser tests and full suite are green.

## Post-Completion Update
1. Update `Todos.md` row 02: `Status=done`, set `Done Date`.
2. Add progress log with changed files and test command summary.
3. Promote row 03 to `in_progress` only when all tests pass.

## Rollback Notes
1. If statement parsing destabilizes expression parsing, rollback parser delta and re-apply incrementally.
