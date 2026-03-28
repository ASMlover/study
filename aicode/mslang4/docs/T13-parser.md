# T13: Parser

**Phase**: 4 - AST & Parser
**Dependencies**: T10 (Scanner), T11 (AST Nodes)
**Estimated Complexity**: High

## Goal

Implement a complete parser that converts a token stream into an AST. Uses a Pratt parser (operator precedence parsing) for expressions and recursive descent for statements.

## Files to Create

| File | Purpose |
|------|---------|
| `src/parser.h` | Parser struct and API |
| `src/parser.c` | Full parser implementation |

## TDD Implementation Cycles

### Cycle 1: Parser Init and Literal Expressions

**RED** — Write failing test:
- Create `tests/unit/test_parser.c`
- Write `test_parser_init()`: call `ms_parser_init(&parser, "")`, verify `hadError == false` and `ms_parser_had_error() == false`
- Write `test_number_literal()`: parse `"42"`, verify returns 1 statement containing `MS_EXPR_LITERAL` with value `42`
- Write `test_string_literal()`: parse `"\"hello\""`, verify string literal node
- Write `test_bool_nil_literals()`: parse `"true"`, `"false"`, `"nil"`, verify correct literal types
- Write `test_grouping()`: parse `"(1 + 2)"`, verify `MS_EXPR_GROUPING` wrapping inner expression
- Expected failure: linker error — `ms_parser_init`, `ms_parser_parse`, `ms_parser_had_error` undefined

**Verify RED**: `cmake --build build 2>&1 | grep "undefined reference"` — link errors for parser symbols

**GREEN** — Minimal implementation:
- Create `src/parser.h` with full struct definitions:
```c
#ifndef MS_PARSER_H
#define MS_PARSER_H

#include "scanner.h"
#include "ast.h"

typedef struct {
    MsToken token;
    char message[256];
} MsParseError;

typedef struct {
    MsScanner scanner;
    MsToken current;
    MsToken previous;
    bool hadError;
    bool panicMode;
    MsParseError lastError;
} MsParser;

void ms_parser_init(MsParser* parser, const char* source);
int ms_parser_parse(MsParser* parser, MsStmt*** outStatements);
bool ms_parser_had_error(const MsParser* parser);

#endif
```
- Create `src/parser.c` implementing:
  - `ms_parser_init()`: init scanner, set `hadError = false`, `panicMode = false`
  - `ms_parser_parse()`: stub that returns 0 statements for empty input
  - Internal: `advance()`, `consume()`, `check()`, `match()` helper functions
  - Expression parsing: `parsePrimary()` handles `MS_TOKEN_NUMBER`, `MS_TOKEN_STRING`, `MS_TOKEN_TRUE`, `MS_TOKEN_FALSE`, `MS_TOKEN_NIL`, and `MS_TOKEN_LEFT_PAREN` (grouping)
  - Statement: wrap each expression in `MS_STMT_EXPR`
- Keep `parseExpr()` calling `parsePrimary()` only (no binary/unary yet)

**Verify GREEN**: `cmake --build build && ./build/test_parser` — literal tests pass

**REFACTOR**: Extract `allocStmt()` and `allocExpr()` helpers for consistent allocation

### Cycle 2: Unary and Binary Arithmetic (Precedence)

**RED** — Write failing test:
- Add `test_unary_negate()`: parse `"-42"`, verify `MS_EXPR_UNARY` with `MS_TOKEN_MINUS` and operand literal 42
- Add `test_unary_not()`: parse `"!true"`, verify `MS_EXPR_UNARY` with `MS_TOKEN_BANG`
- Add `test_binary_add()`: parse `"1 + 2"`, verify `MS_EXPR_BINARY` with `MS_TOKEN_PLUS`
- Add `test_precedence()`: parse `"1 + 2 * 3"`, verify tree is `Binary(PLUS, Literal(1), Binary(STAR, Literal(2), Literal(3)))`
- Add `test_left_associativity()`: parse `"1 - 2 - 3"`, verify `(1-2)-3` grouping
- Expected failure: assertion failure — parser doesn't recognize binary/unary operators, returns wrong AST shape

**Verify RED**: `./build/test_parser` — new tests fail with assertion errors on AST structure

**GREEN** — Minimal implementation:
- Implement Pratt parser infrastructure:
  - Expression parsing by precedence: `parseAssignment`→`parseOr`→`parseAnd`→`parseEquality`→`parseComparison`→`parseTerm`→`parseFactor`→`parseUnary`→`parseCall`→`parsePrimary`
  - Each level: check for prefix token, call next-higher-precedence parser for left operand, then loop on infix tokens at this level
- `parseUnary()`: if `-` or `!`, consume operator, recurse for operand, return `MS_EXPR_UNARY`
- `parseTerm()`: parse left (via `parseFactor`), then loop on `+`/`-`, consuming and building `MS_EXPR_BINARY`
- `parseFactor()`: parse left (via `parseUnary`), then loop on `*`/`/`/`%`, consuming and building `MS_EXPR_BINARY`
- Higher-level functions (`parseComparison` through `parseAssignment`) just delegate to next level for now

**Verify GREEN**: `cmake --build build && ./build/test_parser` — arithmetic and precedence tests pass

**REFACTOR**: Consolidate binary parsing into a generic `parseBinary()` helper parameterized by operator set and next-precedence function

### Cycle 3: Comparison, Equality, and Logical Expressions

**RED** — Write failing test:
- Add `test_comparison()`: parse `"1 < 2"`, `"1 <= 2"`, `"1 > 2"`, `"1 >= 2"`, verify `MS_EXPR_BINARY` with correct operator
- Add `test_equality()`: parse `"1 == 2"`, `"1 != 2"`, verify correct binary nodes
- Add `test_logical_and()`: parse `"true and false"`, verify `MS_EXPR_LOGICAL` with `MS_TOKEN_AND`
- Add `test_logical_or()`: parse `"true or false"`, verify `MS_EXPR_LOGICAL` with `MS_TOKEN_OR`
- Add `test_mixed_precedence()`: parse `"1 + 2 < 3 * 4"`, verify correct tree respecting arithmetic > comparison precedence
- Expected failure: assertion errors — comparison/equality/logical operators not handled

**Verify RED**: `./build/test_parser` — comparison and logical tests fail

**GREEN** — Minimal implementation:
- `parseComparison()`: after calling `parseTerm`, loop on `<`, `<=`, `>`, `>=` tokens, build `MS_EXPR_BINARY`
- `parseEquality()`: after calling `parseComparison`, loop on `==`, `!=` tokens, build `MS_EXPR_BINARY`
- `parseAnd()`: after calling `parseEquality`, loop on `and`, build `MS_EXPR_LOGICAL`
- `parseOr()`: after calling `parseAnd`, loop on `or`, build `MS_EXPR_LOGICAL`
- `parseAssignment()` just delegates to `parseOr` for now

**Verify GREEN**: `cmake --build build && ./build/test_parser` — all expression tests pass

**REFACTOR**: Verify all precedence levels are correct and consistent

### Cycle 4: Variable References and Assignment

**RED** — Write failing test:
- Add `test_variable_reference()`: parse `"x"`, verify `MS_EXPR_VARIABLE` with name "x"
- Add `test_assignment()`: parse `"x = 42"`, verify `MS_EXPR_ASSIGN` with name "x" and value literal 42
- Add `test_compound_expression()`: parse `"x + y"`, verify binary with two variable references
- Expected failure: assertion error — identifiers not parsed as variables

**Verify RED**: `./build/test_parser` — variable tests fail

**GREEN** — Minimal implementation:
- In `parsePrimary()`: handle `MS_TOKEN_IDENTIFIER` — create `MS_EXPR_VARIABLE` node
- `parseAssignment()`: after calling `parseOr`, check if result is `MS_EXPR_VARIABLE` and next token is `=`; if so, consume `=`, parse value expression, return `MS_EXPR_ASSIGN`; otherwise return as-is
- Handle `parseCall()` level between `parseUnary` and `parsePrimary`: for now, just delegate to `parsePrimary`

**Verify GREEN**: `cmake --build build && ./build/test_parser` — variable and assignment tests pass

**REFACTOR**: Ensure assignment is right-associative (`a = b = c` parses as `a = (b = c)`)

### Cycle 5: Variable Declarations and Block Statements

**RED** — Write failing test:
- Add `test_var_decl_no_init()`: parse `"var x"`, verify `MS_STMT_VAR_DECL` with name "x" and no initializer
- Add `test_var_decl_with_init()`: parse `"var x = 42"`, verify `MS_STMT_VAR_DECL` with initializer
- Add `test_block()`: parse `"{ var x = 1\n var y = 2 }"`, verify `MS_STMT_BLOCK` containing two var declarations
- Add `test_empty_block()`: parse `"{}"`, verify block with zero statements
- Expected failure: assertion error — `var` keyword and blocks not handled

**Verify RED**: `./build/test_parser` — declaration and block tests fail

**GREEN** — Minimal implementation:
- Implement `parseDeclaration()`: if `MS_TOKEN_VAR`, call `parseVarDeclaration()`; otherwise call `parseStatement()`
- `parseVarDeclaration()`: consume `var`, consume identifier, if `=` consume and parse expression, consume newline/semicolon, return `MS_STMT_VAR_DECL`
- `parseBlock()`: consume `{`, loop statements until `}`, consume `}`, return `MS_STMT_BLOCK`
- `parseStatement()`: dispatch on token — `{` → block; otherwise → `parseExpressionStatement()`
- `parseExpressionStatement()`: parse expression, consume newline/semicolon, return `MS_STMT_EXPR`
- Update `ms_parser_parse()`: loop `parseDeclaration()` until EOF, collect into dynamic array

**Verify GREEN**: `cmake --build build && ./build/test_parser` — declaration and block tests pass

### Cycle 6: Control Flow Statements

**RED** — Write failing test:
- Add `test_if_statement()`: parse `"if (true) print 1"`, verify `MS_STMT_IF` with condition and then-branch
- Add `test_if_else()`: parse `"if (true) print 1 else print 2"`, verify `MS_STMT_IF` with else-branch
- Add `test_while_statement()`: parse `"while (i < 10) print i"`, verify `MS_STMT_WHILE`
- Add `test_for_statement()`: parse `"for (var i = 0; i < 10; i = i + 1) print i"`, verify `MS_STMT_FOR` with init/condition/increment/body
- Add `test_return_statement()`: parse `"return 42"`, verify `MS_STMT_RETURN` with value
- Add `test_break_continue()`: parse `"break"`, `"continue"`, verify `MS_STMT_BREAK` and `MS_STMT_CONTINUE`
- Expected failure: assertion errors — control flow keywords not handled

**Verify RED**: `./build/test_parser` — control flow tests fail

**GREEN** — Minimal implementation:
- Extend `parseStatement()` dispatch:
  - `MS_TOKEN_IF` → `parseIfStatement()`: consume `if`, `(`, parse expression, `)`, parse statement, optionally consume `else` and parse else-branch, return `MS_STMT_IF`
  - `MS_TOKEN_WHILE` → `parseWhileStatement()`: consume `while`, `(`, parse expression, `)`, parse statement, return `MS_STMT_WHILE`
  - `MS_TOKEN_FOR` → `parseForStatement()`: consume `for`, `(`, parse init (var decl or expr), `;`, parse condition, `;`, parse increment, `)`, parse body, return `MS_STMT_FOR`
  - `MS_TOKEN_RETURN` → `parseReturnStatement()`: consume `return`, parse expression (optional), consume newline, return `MS_STMT_RETURN`
  - `MS_TOKEN_BREAK` → consume `break`, return `MS_STMT_BREAK`
  - `MS_TOKEN_CONTINUE` → consume `continue`, return `MS_STMT_CONTINUE`

**Verify GREEN**: `cmake --build build && ./build/test_parser` — all control flow tests pass

**REFACTOR**: Extract common "parse parenthesized expression" helper

### Cycle 7: Function Declarations

**RED** — Write failing test:
- Add `test_function_decl()`: parse `"fn add(a, b) { return a + b }"`, verify `MS_STMT_FUNC_DECL` with name "add", params ["a","b"], body containing return statement
- Add `test_function_no_params()`: parse `"fn foo() { print 1 }"`, verify zero params
- Add `test_function_empty_body()`: parse `"fn noop() {}"`, verify empty body
- Expected failure: assertion error — `fn` keyword not handled

**Verify RED**: `./build/test_parser` — function tests fail

**GREEN** — Minimal implementation:
- Extend `parseDeclaration()` dispatch: `MS_TOKEN_FN` → `parseFuncDeclaration()`
- `parseFuncDeclaration()`: consume `fn`, consume identifier (name), consume `(`, loop comma-separated identifiers for params, consume `)`, parse block body, return `MS_STMT_FUNC_DECL`
- `parsePrimary()` / `parseCall()`: handle function calls — after primary expression, if `(` follows, parse comma-separated arguments, create `MS_EXPR_CALL` node

**Verify GREEN**: `cmake --build build && ./build/test_parser` — function tests pass

**REFACTOR**: Share parameter-list parsing logic with call expression argument-list parsing

### Cycle 8: Class Declarations and Import Statements

**RED** — Write failing test:
- Add `test_class_decl()`: parse `"class Foo { fn bar() { print 1 } }"`, verify `MS_STMT_CLASS_DECL` with name "Foo" and one method
- Add `test_class_inheritance()`: parse `"class Child < Parent { }"`, verify superclass "Parent"
- Add `test_import_simple()`: parse `"import math"`, verify `MS_STMT_IMPORT` with module "math"
- Add `test_import_from()`: parse `"from math import sqrt"`, verify `MS_STMT_IMPORT` with module "math" and items ["sqrt"]
- Add `test_this_super()`: parse `"this.x"`, `"super.greet()"`, verify `MS_EXPR_GET` on `this`/`super` node
- Expected failure: assertion errors — class/import/this/super not handled

**Verify RED**: `./build/test_parser` — class, import, this/super tests fail

**GREEN** — Minimal implementation:
- Extend `parseDeclaration()`: `MS_TOKEN_CLASS` → `parseClassDeclaration()`
- `parseClassDeclaration()`: consume `class`, consume name, if `<` consume superclass name, consume `{`, loop method declarations, consume `}`, return `MS_STMT_CLASS_DECL`
- Extend `parseDeclaration()`: `MS_TOKEN_IMPORT` → `parseImportStatement()`
- `parseImportStatement()`: consume `import`, consume module name; or `from` module `import` items, return `MS_STMT_IMPORT`
- `parsePrimary()`: handle `MS_TOKEN_THIS` → `MS_EXPR_THIS`; `MS_TOKEN_SUPER` → `MS_EXPR_SUPER`
- `parseCall()` level: handle `.name` (get), `.name = expr` (set), `[index]` (subscript), `[index] = expr` (subscript assign)
- List literal: `[elem1, elem2]` → `MS_EXPR_LIST`

**Verify GREEN**: `cmake --build build && ./build/test_parser` — all class/import/get/set tests pass

**REFACTOR**: Verify all expression types are handled, consolidate property access parsing

### Cycle 9: Error Recovery

**RED** — Write failing test:
- Add `test_error_missing_semicolon()`: parse `"var x = 42 var y = 1"`, verify error reported, both declarations still parsed
- Add `test_error_unclosed_brace()`: parse `"{ var x = 1"`, verify error reported
- Add `test_error_invalid_token()`: parse `"var 42 = x"`, verify error with line/column info
- Add `test_error_multiple()`: parse `"var x =\nvar y = 1 + \nprint 1"`, verify multiple errors collected, parser doesn't crash
- Add `test_had_error_flag()`: verify `ms_parser_had_error()` returns true after any error
- Expected failure: assertion error — error recovery not functional, parser crashes on invalid input

**Verify RED**: `./build/test_parser` — error tests fail or crash

**GREEN** — Minimal implementation:
- Implement `synchronize()`: skip tokens until statement boundary (`MS_TOKEN_NEWLINE`, `MS_TOKEN_RIGHT_BRACE`, `MS_TOKEN_CLASS`, `MS_TOKEN_FN`, `MS_TOKEN_VAR`, `MS_TOKEN_FOR`, `MS_TOKEN_IF`, `MS_TOKEN_WHILE`, `MS_TOKEN_PRINT`, `MS_TOKEN_RETURN`)
- In `parseDeclaration()` and `parseStatement()`: on error, call `synchronize()` before continuing
- `consume()`: if current token doesn't match expected, set `hadError = true`, store error in `lastError`, and if not in `panicMode`, enter panic mode and call `synchronize()`
- Ensure `ms_parser_parse()` continues looping even after errors, returning all successfully parsed statements

**Verify GREEN**: `cmake --build build && ./build/test_parser` — error recovery tests pass, parser doesn't crash

**REFACTOR**: Add line/column info to all error messages; ensure error messages are descriptive

## Acceptance Criteria

- [ ] Parse `"var x = 42"` → VarDecl AST node
- [ ] Parse `"1 + 2 * 3"` → Binary(PLUS, Literal(1), Binary(STAR, Literal(2), Literal(3)))
- [ ] Parse `"if (true) print 1 else print 2"` → If AST with then/else branches
- [ ] Parse `"fn add(a, b) { return a + b }"` → Function AST
- [ ] Parse `"class Foo { fn bar() { ... } }"` → Class AST
- [ ] Parse `"import math"` → Import AST
- [ ] Parse `"from math import sqrt"` → Import AST with items
- [ ] Error recovery: invalid syntax doesn't crash, reports error
- [ ] `ms_parser_had_error()` returns true after parsing invalid input
- [ ] Complex programs parse correctly

## Notes

- The parser follows a Pratt (top-down operator precedence) approach for expressions and recursive descent for statements, matching the clox architecture
- `ms_parser_parse()` returns count of parsed statements (-1 on fatal error). Caller must free via `ms_stmt_list_free()`
- Expression types handled: literals, variables, unary, binary, logical, grouping, assignment, call, get/set, this/super, list, subscript
- Statement types handled: var/function/class/import declarations, block, if, while, for, return, break, continue, expression
