# T11: AST Nodes

**Phase**: 4 - AST & Parser
**Dependencies**: T06 (Token Types)
**Estimated Complexity**: Medium

## Goal

Define all AST node types for expressions and statements, plus create/free functions. This is the output of the parser and the input to the compiler.

## Files to Create

| File | Purpose |
|------|---------|
| `src/ast.h` | AST type enums, node structs, create/free declarations |
| `src/ast.c` | Node creation and recursive free |

## TDD Implementation Cycles

### Cycle 1: Expression Create/Free (Literal)

**RED** — Write failing test:

Create `tests/unit/test_ast.c`. Write a test function `test_expr_literal` that:
- Creates a literal expression via `ms_expr_create(MS_EXPR_LITERAL)`
- Verifies it is non-NULL and has the correct type
- Sets literal type and value
- Frees it without crash

```c
// tests/unit/test_ast.c (initial skeleton)
#include "ast.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

static void test_expr_literal(void) {
    MsExpr* expr = ms_expr_create(MS_EXPR_LITERAL);
    assert(expr != NULL);
    assert(expr->type == MS_EXPR_LITERAL);

    expr->literal.type = MS_LITERAL_NUMBER;
    expr->literal.value.number = 42.0;

    MsExpr* expr_nil = ms_expr_create(MS_EXPR_LITERAL);
    assert(expr_nil != NULL);
    expr_nil->literal.type = MS_LITERAL_NIL;

    MsExpr* expr_bool = ms_expr_create(MS_EXPR_LITERAL);
    assert(expr_bool != NULL);
    expr_bool->literal.type = MS_LITERAL_BOOL;
    expr_bool->literal.value.boolean = true;

    ms_expr_free(expr);
    ms_expr_free(expr_nil);
    ms_expr_free(expr_bool);
    printf("  test_expr_literal PASSED\n");
}

int main(void) {
    printf("Running AST tests...\n");
    test_expr_literal();
    printf("All AST tests passed.\n");
    return 0;
}
```

**Verify RED**: `gcc -I src -o build/test_ast tests/unit/test_ast.c src/ast.c` → compilation error: `ast.h: No such file or directory`

**GREEN** — Minimal implementation:

Create `src/ast.h` with expression types and literal support:

```c
#ifndef MS_AST_H
#define MS_AST_H

#include "common.h"
#include "token.h"

typedef enum {
    MS_EXPR_ASSIGN, MS_EXPR_BINARY, MS_EXPR_CALL, MS_EXPR_GET,
    MS_EXPR_GROUPING, MS_EXPR_LITERAL, MS_EXPR_LOGICAL, MS_EXPR_SET,
    MS_EXPR_SUPER, MS_EXPR_THIS, MS_EXPR_UNARY, MS_EXPR_VARIABLE,
    MS_EXPR_LIST, MS_EXPR_SUBSCRIPT
} MsExprType;

typedef enum { MS_LITERAL_NIL, MS_LITERAL_BOOL, MS_LITERAL_NUMBER, MS_LITERAL_STRING } MsLiteralType;

typedef struct {
    MsLiteralType type;
    union { bool boolean; double number; struct { const char* start; int length; } string_view; } value;
} MsLiteralValue;

typedef struct MsExpr {
    MsExprType type;
    union {
        struct { struct MsExpr* target; MsToken name; struct MsExpr* value; } assign;
        struct { MsToken op; struct MsExpr* left; struct MsExpr* right; } binary;
        struct { struct MsExpr* callee; struct MsExpr** args; int arg_count; MsToken paren; } call;
        struct { struct MsExpr* object; MsToken name; } get;
        struct { struct MsExpr* expression; } grouping;
        MsLiteralValue literal;
        struct { MsToken op; struct MsExpr* left; struct MsExpr* right; } logical;
        struct { struct MsExpr* object; MsToken name; struct MsExpr* value; } set;
        struct { MsToken keyword; MsToken method; } super;
        MsToken this_expr;
        struct { MsToken op; struct MsExpr* operand; } unary;
        MsToken variable;
        struct { struct MsExpr** elements; int count; } list;
        struct { struct MsExpr* object; struct MsExpr* index; MsToken bracket; } subscript;
    };
} MsExpr;

MsExpr* ms_expr_create(MsExprType type);
void ms_expr_free(MsExpr* expr);

/* Forward declare statement types (Cycle 3) */
typedef struct MsStmt MsStmt;
typedef struct { MsToken name; MsToken alias; } MsImportItem;

void ms_stmt_free(MsStmt* stmt);
void ms_stmt_list_free(MsStmt** stmts, int count);

#endif
```

Create `src/ast.c`:

```c
#include "ast.h"
#include <stdlib.h>
#include <string.h>

MsExpr* ms_expr_create(MsExprType type) {
    MsExpr* expr = MS_ALLOCATE(MsExpr, 1);
    memset(expr, 0, sizeof(MsExpr));
    expr->type = type;
    return expr;
}

void ms_expr_free(MsExpr* expr) {
    if (expr == NULL) return;
    switch (expr->type) {
        case MS_EXPR_ASSIGN:
            ms_expr_free(expr->assign.target);
            ms_expr_free(expr->assign.value);
            break;
        case MS_EXPR_BINARY:
            ms_expr_free(expr->binary.left);
            ms_expr_free(expr->binary.right);
            break;
        case MS_EXPR_CALL:
            ms_expr_free(expr->call.callee);
            if (expr->call.args) MS_FREE(MsExpr*, expr->call.args, expr->call.arg_count);
            break;
        case MS_EXPR_GET:
            ms_expr_free(expr->get.object);
            break;
        case MS_EXPR_GROUPING:
            ms_expr_free(expr->grouping.expression);
            break;
        case MS_EXPR_LITERAL: break;
        case MS_EXPR_LOGICAL:
            ms_expr_free(expr->logical.left);
            ms_expr_free(expr->logical.right);
            break;
        case MS_EXPR_SET:
            ms_expr_free(expr->set.object);
            ms_expr_free(expr->set.value);
            break;
        case MS_EXPR_SUPER: break;
        case MS_EXPR_THIS: break;
        case MS_EXPR_UNARY:
            ms_expr_free(expr->unary.operand);
            break;
        case MS_EXPR_VARIABLE: break;
        case MS_EXPR_LIST:
            if (expr->list.elements) {
                for (int i = 0; i < expr->list.count; i++) ms_expr_free(expr->list.elements[i]);
                MS_FREE(MsExpr*, expr->list.elements, expr->list.count);
            }
            break;
        case MS_EXPR_SUBSCRIPT:
            ms_expr_free(expr->subscript.object);
            ms_expr_free(expr->subscript.index);
            break;
    }
    MS_FREE(MsExpr, expr, 1);
}
```

`ms_expr_create(type)`: `MS_ALLOCATE(MsExpr, 1)`, zero-initialize with `memset`, set type.
`ms_expr_free(expr)`: Recursively free child expressions based on type, then `MS_FREE(MsExpr, expr, 1)`.

**Verify GREEN**: `gcc -I src -o build/test_ast tests/unit/test_ast.c src/ast.c && ./build/test_ast` → test passes

**REFACTOR**: None needed.

---

### Cycle 2: All Expression Types (Binary, Unary, Grouping, Variable)

**RED** — Write failing test:

Add `test_expr_types` to `test_ast.c`:

```c
static void test_expr_types(void) {
    /* Binary: 1 + 2 */
    MsExpr* left = ms_expr_create(MS_EXPR_LITERAL);
    left->literal.type = MS_LITERAL_NUMBER;
    left->literal.value.number = 1.0;

    MsExpr* right = ms_expr_create(MS_EXPR_LITERAL);
    right->literal.type = MS_LITERAL_NUMBER;
    right->literal.value.number = 2.0;

    MsExpr* binary = ms_expr_create(MS_EXPR_BINARY);
    binary->binary.left = left;
    binary->binary.right = right;
    assert(binary->type == MS_EXPR_BINARY);

    /* Unary: -expr */
    MsExpr* operand = ms_expr_create(MS_EXPR_LITERAL);
    operand->literal.type = MS_LITERAL_NUMBER;
    operand->literal.value.number = 3.0;
    MsExpr* unary = ms_expr_create(MS_EXPR_UNARY);
    unary->unary.operand = operand;

    /* Grouping */
    MsExpr* inner = ms_expr_create(MS_EXPR_LITERAL);
    inner->literal.type = MS_LITERAL_NUMBER;
    inner->literal.value.number = 4.0;
    MsExpr* grouping = ms_expr_create(MS_EXPR_GROUPING);
    grouping->grouping.expression = inner;

    /* Variable */
    MsExpr* var = ms_expr_create(MS_EXPR_VARIABLE);
    assert(var->type == MS_EXPR_VARIABLE);

    /* Free all (recursive) */
    ms_expr_free(binary);
    ms_expr_free(unary);
    ms_expr_free(grouping);
    ms_expr_free(var);
    printf("  test_expr_types PASSED\n");
}
```

**Verify RED**: Should compile and pass since expression types and free were implemented in Cycle 1. This is a verification cycle.

**Verify GREEN**: Build and run — both tests pass.

**REFACTOR**: None needed.

---

### Cycle 3: Statement Create/Free (Expression and Var)

**RED** — Write failing test:

Add `test_stmt_basic` to `test_ast.c`:

```c
static void test_stmt_basic(void) {
    /* Expression statement */
    MsStmt* expr_stmt = ms_stmt_create(MS_STMT_EXPRESSION);
    assert(expr_stmt != NULL);
    assert(expr_stmt->type == MS_STMT_EXPRESSION);

    MsExpr* lit = ms_expr_create(MS_EXPR_LITERAL);
    lit->literal.type = MS_LITERAL_NUMBER;
    lit->literal.value.number = 1.0;
    expr_stmt->expression = lit;

    /* Var statement */
    MsStmt* var_stmt = ms_stmt_create(MS_STMT_VAR);
    assert(var_stmt != NULL);
    assert(var_stmt->type == MS_STMT_VAR);

    ms_stmt_free(expr_stmt);
    ms_stmt_free(var_stmt);
    printf("  test_stmt_basic PASSED\n");
}
```

**Verify RED**: Build fails — `MsStmt`, `ms_stmt_create` undeclared.

**GREEN** — Minimal implementation:

Add to `src/ast.h`:

```c
typedef enum {
    MS_STMT_BLOCK, MS_STMT_CLASS, MS_STMT_EXPRESSION, MS_STMT_FUNCTION,
    MS_STMT_IF, MS_STMT_IMPORT, MS_STMT_RETURN, MS_STMT_VAR,
    MS_STMT_WHILE, MS_STMT_FOR, MS_STMT_BREAK, MS_STMT_CONTINUE
} MsStmtType;

struct MsStmt {
    MsStmtType type;
    union {
        struct { MsStmt** stmts; int count; } block;
        struct { MsToken name; MsStmt** methods; int method_count; struct MsExpr* superclass; } class_stmt;
        struct MsExpr* expression;
        struct { MsToken name; MsToken* params; int param_count; MsStmt** body; int body_count; MsString* docstring; } function;
        struct { struct MsExpr* condition; MsStmt* then_branch; MsStmt* else_branch; } if_stmt;
        struct { MsImportItem* items; int item_count; MsToken module_path; } import;
        struct { MsToken keyword; struct MsExpr* value; } return_stmt;
        struct { MsToken name; struct MsExpr* initializer; } var;
        struct { struct MsExpr* condition; MsStmt* body; } while_stmt;
        struct { MsToken var_name; struct MsExpr* iterator; struct MsExpr* collection; MsStmt* body; } for_stmt;
        MsToken break_stmt;
        MsToken continue_stmt;
    };
};
```

Add declaration:
```c
MsStmt* ms_stmt_create(MsStmtType type);
```

Add to `src/ast.c`:

```c
MsStmt* ms_stmt_create(MsStmtType type) {
    MsStmt* stmt = MS_ALLOCATE(MsStmt, 1);
    memset(stmt, 0, sizeof(MsStmt));
    stmt->type = type;
    return stmt;
}
```

Implement `ms_stmt_free` (at minimum handle EXPRESSION and VAR):

```c
void ms_stmt_free(MsStmt* stmt) {
    if (stmt == NULL) return;
    switch (stmt->type) {
        case MS_STMT_EXPRESSION:
            ms_expr_free(stmt->expression);
            break;
        case MS_STMT_VAR:
            ms_expr_free(stmt->var.initializer);
            break;
        /* Other types handled in Cycle 4 */
        default: break;
    }
    MS_FREE(MsStmt, stmt, 1);
}
```

**Verify GREEN**: Build and run — all three tests pass.

**REFACTOR**: None needed.

---

### Cycle 4: All Statement Types (Block, Function, Class, If, While, For, Import)

**RED** — Write failing test:

Add `test_stmt_types` to `test_ast.c`:

```c
static void test_stmt_types(void) {
    /* Block with 3 child statements */
    MsStmt* block = ms_stmt_create(MS_STMT_BLOCK);
    block->block.stmts = MS_ALLOCATE(MsStmt*, 3);
    block->block.count = 3;
    for (int i = 0; i < 3; i++) {
        block->block.stmts[i] = ms_stmt_create(MS_STMT_EXPRESSION);
        MsExpr* lit = ms_expr_create(MS_EXPR_LITERAL);
        lit->literal.type = MS_LITERAL_NUMBER;
        lit->literal.value.number = (double)i;
        block->block.stmts[i]->expression = lit;
    }
    assert(block->type == MS_STMT_BLOCK);

    /* Function with params and body */
    MsStmt* func = ms_stmt_create(MS_STMT_FUNCTION);
    func->function.param_count = 2;
    func->function.params = MS_ALLOCATE(MsToken, 2);
    func->function.body_count = 1;
    func->function.body = MS_ALLOCATE(MsStmt*, 1);
    func->function.body[0] = ms_stmt_create(MS_STMT_EXPRESSION);
    assert(func->type == MS_STMT_FUNCTION);

    /* If statement */
    MsStmt* if_stmt = ms_stmt_create(MS_STMT_IF);
    assert(if_stmt->type == MS_STMT_IF);

    /* While statement */
    MsStmt* while_stmt = ms_stmt_create(MS_STMT_WHILE);
    assert(while_stmt->type == MS_STMT_WHILE);

    /* Import statement */
    MsStmt* import = ms_stmt_create(MS_STMT_IMPORT);
    assert(import->type == MS_STMT_IMPORT);

    /* Free all recursively */
    ms_stmt_free(block);
    ms_stmt_free(func);
    ms_stmt_free(if_stmt);
    ms_stmt_free(while_stmt);
    ms_stmt_free(import);
    printf("  test_stmt_types PASSED\n");
}
```

**Verify RED**: Build and run — should compile but may leak memory or crash if `ms_stmt_free` doesn't handle BLOCK and FUNCTION properly.

**GREEN** — Update `ms_stmt_free` to handle all statement types:

```c
void ms_stmt_free(MsStmt* stmt) {
    if (stmt == NULL) return;
    switch (stmt->type) {
        case MS_STMT_BLOCK:
            if (stmt->block.stmts) {
                for (int i = 0; i < stmt->block.count; i++) ms_stmt_free(stmt->block.stmts[i]);
                MS_FREE(MsStmt*, stmt->block.stmts, stmt->block.count);
            }
            break;
        case MS_STMT_CLASS:
            ms_expr_free(stmt->class_stmt.superclass);
            if (stmt->class_stmt.methods) {
                for (int i = 0; i < stmt->class_stmt.method_count; i++)
                    ms_stmt_free(stmt->class_stmt.methods[i]);
                MS_FREE(MsStmt*, stmt->class_stmt.methods, stmt->class_stmt.method_count);
            }
            break;
        case MS_STMT_EXPRESSION:
            ms_expr_free(stmt->expression);
            break;
        case MS_STMT_FUNCTION:
            if (stmt->function.params) MS_FREE(MsToken, stmt->function.params, stmt->function.param_count);
            if (stmt->function.body) {
                for (int i = 0; i < stmt->function.body_count; i++) ms_stmt_free(stmt->function.body[i]);
                MS_FREE(MsStmt*, stmt->function.body, stmt->function.body_count);
            }
            break;
        case MS_STMT_IF:
            ms_expr_free(stmt->if_stmt.condition);
            ms_stmt_free(stmt->if_stmt.then_branch);
            ms_stmt_free(stmt->if_stmt.else_branch);
            break;
        case MS_STMT_IMPORT:
            if (stmt->import.items) MS_FREE(MsImportItem, stmt->import.items, stmt->import.item_count);
            break;
        case MS_STMT_RETURN:
            ms_expr_free(stmt->return_stmt.value);
            break;
        case MS_STMT_VAR:
            ms_expr_free(stmt->var.initializer);
            break;
        case MS_STMT_WHILE:
            ms_expr_free(stmt->while_stmt.condition);
            ms_stmt_free(stmt->while_stmt.body);
            break;
        case MS_STMT_FOR:
            ms_expr_free(stmt->for_stmt.iterator);
            ms_expr_free(stmt->for_stmt.collection);
            ms_stmt_free(stmt->for_stmt.body);
            break;
        case MS_STMT_BREAK: break;
        case MS_STMT_CONTINUE: break;
    }
    MS_FREE(MsStmt, stmt, 1);
}
```

For BLOCK: free statement array. For FUNCTION: free params array, free body statements. For CLASS: free methods array.

**Verify GREEN**: Build and run — all four tests pass.

**REFACTOR**: Run with address sanitizer to verify no leaks.

---

### Cycle 5: Complex Nested AST Tree Free

**RED** — Write failing test:

Add `test_nested_ast` to `test_ast.c`:

```c
static void test_nested_ast(void) {
    /* Build: 1 + 2 * 3 → Binary(PLUS, Literal(1), Binary(STAR, Literal(2), Literal(3))) */
    MsExpr* lit1 = ms_expr_create(MS_EXPR_LITERAL);
    lit1->literal.type = MS_LITERAL_NUMBER;
    lit1->literal.value.number = 1.0;

    MsExpr* lit2 = ms_expr_create(MS_EXPR_LITERAL);
    lit2->literal.type = MS_LITERAL_NUMBER;
    lit2->literal.value.number = 2.0;

    MsExpr* lit3 = ms_expr_create(MS_EXPR_LITERAL);
    lit3->literal.type = MS_LITERAL_NUMBER;
    lit3->literal.value.number = 3.0;

    MsExpr* mul = ms_expr_create(MS_EXPR_BINARY);
    mul->binary.left = lit2;
    mul->binary.right = lit3;

    MsExpr* add = ms_expr_create(MS_EXPR_BINARY);
    add->binary.left = lit1;
    add->binary.right = mul;

    /* Wrap in expression statement */
    MsStmt* stmt = ms_stmt_create(MS_STMT_EXPRESSION);
    stmt->expression = add;

    ms_stmt_free(stmt);
    printf("  test_nested_ast PASSED\n");
}
```

**Verify RED**: Should compile and pass. This is a verification cycle for recursive free of complex trees.

**Verify GREEN**: Build and run — all five tests pass.

**REFACTOR**: None needed.

---

### Cycle 6: Statement List Free

**RED** — Write failing test:

Add `test_stmt_list_free` to `test_ast.c`:

```c
static void test_stmt_list_free(void) {
    /* Empty list */
    ms_stmt_list_free(NULL, 0);

    /* List with statements */
    MsStmt** stmts = MS_ALLOCATE(MsStmt*, 3);
    for (int i = 0; i < 3; i++) {
        stmts[i] = ms_stmt_create(MS_STMT_EXPRESSION);
        MsExpr* lit = ms_expr_create(MS_EXPR_LITERAL);
        lit->literal.type = MS_LITERAL_NUMBER;
        lit->literal.value.number = (double)i;
        stmts[i]->expression = lit;
    }
    ms_stmt_list_free(stmts, 3);
    printf("  test_stmt_list_free PASSED\n");
}
```

**Verify RED**: Build fails — `ms_stmt_list_free` not yet implemented (declared but not defined).

**GREEN** — Minimal implementation:

Add to `src/ast.c`:

```c
void ms_stmt_list_free(MsStmt** stmts, int count) {
    if (stmts == NULL) return;
    for (int i = 0; i < count; i++) ms_stmt_free(stmts[i]);
    MS_FREE(MsStmt*, stmts, count);
}
```

Loop and call `ms_stmt_free` on each, then free the array.

**Verify GREEN**: Build and run — all six tests pass.

**REFACTOR**: Run with address sanitizer to verify complete cleanup.

## Acceptance Criteria

- [ ] `ms_expr_create(MS_EXPR_LITERAL)` returns non-NULL with correct type
- [ ] `ms_stmt_create(MS_STMT_VAR)` returns non-NULL with correct type
- [ ] `ms_expr_free()` on a literal expression doesn't crash
- [ ] `ms_stmt_free()` on a block with 3 child statements frees all recursively
- [ ] `ms_stmt_list_free()` handles empty list (count=0, stmts=NULL)
- [ ] Complex AST tree (nested binary expressions) frees completely without leak
- [ ] All expression types and statement types can be created

## Notes

- Child arrays (e.g., `call.args`, `function.body`) are allocated with `MS_ALLOCATE` and freed with `MS_FREE`.
- The `MsLiteralValue.string_view` field stores a pointer into the source code (start + length), not an owned string. No deallocation needed for string literals in the AST.
- The `MsStmt.function.docstring` field references an `MsString*` from the object system; it is not freed by `ms_stmt_free` since it's owned by the string intern table.
- All structs use tagged unions for polymorphism, consistent with the VM's overall design.
