# T14: Compiler - Basic

**Phase**: 6 - Compiler & VM (Basic)
**Dependencies**: T12 (Bytecode Chunk), T13 (Parser)
**Estimated Complexity**: High

## Goal

Implement a single-pass compiler that walks AST and emits bytecode. This task covers basic expressions, variables, and control flow statements. Functions, classes, and closures are deferred to later tasks.

## Files to Create

| File | Purpose |
|------|---------|
| `src/compiler.h` | Compiler struct, state, and API |
| `src/compiler.c` | Compiler implementation (basic subset) |

## TDD Implementation Cycles

### Cycle 1: Compiler Init and Literal Expressions

**RED** — Write failing test:
- Create `tests/unit/test_compiler.c`
- Write `test_compiler_init_free()`: call `ms_compiler_init(&compiler)`, then `ms_compiler_free(&compiler)`, verify no crash/leak
- Write `test_compile_number()`: compile `"42"`, get `MsFunction*`, verify chunk contains `MS_OP_CONSTANT` pointing to `MS_VALUE_NUMBER(42)`, followed by `MS_OP_POP` and `MS_OP_RETURN`
- Write `test_compile_bool()`: compile `"true"`, verify `MS_OP_TRUE`; compile `"false"`, verify `MS_OP_FALSE`; compile `"nil"`, verify `MS_OP_NIL`
- Write `test_compile_string()`: compile `"\"hello\""`, verify `MS_OP_CONSTANT` pointing to string value
- Expected failure: linker error — `ms_compiler_init`, `ms_compiler_free`, `ms_compiler_compile` undefined

**Verify RED**: `cmake --build build 2>&1 | grep "undefined reference"` — link errors for compiler symbols

**GREEN** — Minimal implementation:
- Create `src/compiler.h` with full struct definitions:
```c
#ifndef MS_COMPILER_H
#define MS_COMPILER_H

#include "chunk.h"
#include "parser.h"

typedef enum { MS_FUNC_SCRIPT, MS_FUNC_FUNCTION, MS_FUNC_METHOD, MS_FUNC_INITIALIZER } MsFunctionType;

typedef struct MsLocal { MsToken name; int depth; bool isCaptured; } MsLocal;
typedef struct MsCompilerUpvalue { int index; bool isLocal; } MsCompilerUpvalue;

typedef struct MsCompilerState {
    struct MsCompilerState* enclosing;
    MsFunction* function;
    MsFunctionType type;
    MsLocal locals[MS_MAX_LOCALS];
    int localCount;
    int scopeDepth;
    MsCompilerUpvalue upvalues[MS_MAX_UPVALUES];
    int upvalueCount;
    int* breakJumps; int breakJumpCount; int breakJumpCapacity;
    int* continueJumps; int continueJumpCount; int continueJumpCapacity;
    int loopStart;
} MsCompilerState;

typedef struct { MsParser parser; MsCompilerState* current; } MsCompiler;

void ms_compiler_init(MsCompiler* compiler);
void ms_compiler_free(MsCompiler* compiler);
MsFunction* ms_compiler_compile(MsCompiler* compiler, const char* source);
void ms_compiler_mark_roots(MsCompiler* compiler);

#endif
```
- Update `src/object.h`/`src/object.c` to fully implement `MsFunction` with `MsChunk chunk` field (replace any placeholder)
- Create `src/compiler.c` implementing:
  - `ms_compiler_init()`: zero-init compiler struct
  - `ms_compiler_free()`: free compiler state chain, close parser
  - `ms_compiler_compile()`: init parser, parse source into AST, init compiler state, create top-level `MsFunction` (type `MS_FUNC_SCRIPT`), walk AST compiling each statement, emit `MS_OP_RETURN` at end, return compiled function
  - Internal emit helpers: `emitByte()`, `emitBytes()`, `emitConstant()`, `emitReturn()`
  - `makeConstant()`: add value to current chunk's constant pool, return index
  - `compileExpr()`: dispatch on AST expression type — `MS_EXPR_LITERAL` → emit `MS_OP_CONSTANT`/`MS_OP_NIL`/`MS_OP_TRUE`/`MS_OP_FALSE`
  - `compileStmt()`: dispatch on `MS_STMT_EXPR` → compile expression, emit `MS_OP_POP`

**Verify GREEN**: `cmake --build build && ./build/test_compiler` — init and literal tests pass

**REFACTOR**: Ensure `MsFunction` is properly freed via `ms_object_free()` chain

### Cycle 2: Unary and Binary Arithmetic

**RED** — Write failing test:
- Add `test_compile_unary_negate()`: compile `"-42"`, verify chunk: `OP_CONSTANT(42)`, `OP_NEGATE`, `OP_POP`, `OP_RETURN`
- Add `test_compile_unary_not()`: compile `"!true"`, verify: `OP_TRUE`, `OP_NOT`, `OP_POP`, `OP_RETURN`
- Add `test_compile_add()`: compile `"1 + 2"`, verify: `OP_CONSTANT(1)`, `OP_CONSTANT(2)`, `OP_ADD`, `OP_POP`, `OP_RETURN`
- Add `test_compile_arithmetic()`: compile `"1 + 2 * 3 - 4 / 5 % 6"`, verify all ops emitted in correct order (respecting precedence from AST)
- Expected failure: assertion failure — unary/binary expressions not compiled

**Verify RED**: `./build/test_compiler` — arithmetic tests fail with wrong bytecode

**GREEN** — Minimal implementation:
- Extend `compileExpr()`:
  - `MS_EXPR_UNARY`: compile operand, then emit `MS_OP_NEGATE` (for `-`) or `MS_OP_NOT` (for `!`)
  - `MS_EXPR_BINARY`: compile left, compile right, then emit corresponding opcode (`MS_OP_ADD`, `MS_OP_SUBTRACT`, `MS_OP_MULTIPLY`, `MS_OP_DIVIDE`, `MS_OP_MODULO`)
- Opcode-to-token mapping for binary ops: `+`→`ADD`, `-`→`SUBTRACT`, `*`→`MULTIPLY`, `/`→`DIVIDE`, `%`→`MODULO`

**Verify GREEN**: `cmake --build build && ./build/test_compiler` — arithmetic tests pass

**REFACTOR**: Use a lookup table or switch for operator→opcode mapping to avoid repetition

### Cycle 3: Comparison, Equality, and Logical Expressions

**RED** — Write failing test:
- Add `test_compile_comparison()`: compile `"1 < 2"`, verify: `OP_CONSTANT(1)`, `OP_CONSTANT(2)`, `OP_LESS`
- Add `test_compile_equality()`: compile `"1 == 2"`, verify `OP_EQUAL`; compile `"1 != 2"`, verify `OP_NOT` after `OP_EQUAL`
- Add `test_compile_logical_and()`: compile `"true and false"`, verify: `OP_TRUE`, `OP_JUMP_IF_FALSE(offset)`, `OP_POP`, `OP_FALSE`, `OP_AND` pattern (or short-circuit: `OP_JUMP_IF_FALSE`)
- Add `test_compile_logical_or()`: compile `"true or false"`, verify short-circuit jump pattern
- Expected failure: wrong opcodes or missing jump instructions

**Verify RED**: `./build/test_compiler` — comparison/logical tests fail

**GREEN** — Minimal implementation:
- `MS_EXPR_BINARY` for comparison: emit `MS_OP_LESS`, `MS_OP_GREATER`, `MS_OP_LESS_EQUAL`, `MS_OP_GREATER_EQUAL`, `MS_OP_EQUAL`; for `!=`, emit `MS_OP_EQUAL` then `MS_OP_NOT`
- Implement `emitJump()` and `patchJump()` for forward jumps
- `MS_EXPR_LOGICAL`:
  - `and`: compile left, emit `MS_OP_JUMP_IF_FALSE` (skip right if left is falsy), `MS_OP_POP`, compile right
  - `or`: compile left, emit `MS_OP_JUMP_IF_TRUE` (skip right if left is truthy), `MS_OP_POP`, compile right
- Logical ops use two-byte signed offset for jump destination

**Verify GREEN**: `cmake --build build && ./build/test_compiler` — comparison and logical tests pass

**REFACTOR**: Verify jump offset calculation is correct for all cases

### Cycle 4: Global Variables

**RED** — Write failing test:
- Add `test_compile_var_decl()`: compile `"var x = 42"`, verify: `OP_CONSTANT(42)`, `OP_DEFINE_GLOBAL("x")`
- Add `test_compile_var_decl_no_init()`: compile `"var x"`, verify: `OP_NIL`, `OP_DEFINE_GLOBAL("x")`
- Add `test_compile_get_global()`: compile `"print x"` (where x is defined), verify `OP_GET_GLOBAL("x")`
- Add `test_compile_set_global()`: compile `"x = 42"`, verify: `OP_CONSTANT(42)`, `OP_SET_GLOBAL("x")`
- Add `test_compile_undefined_var()`: compile `"print undefined_var"`, verify compiler reports error
- Add `test_compile_redeclare()`: compile `"var x = 1\nvar x = 2"`, verify compiler reports error
- Expected failure: assertion failures — variable opcodes not emitted

**Verify RED**: `./build/test_compiler` — variable tests fail

**GREEN** — Minimal implementation:
- `identifierConstant()`: add token's lexeme string to constant pool, return index
- Compile `MS_STMT_VAR_DECL`: compile initializer (or emit `MS_OP_NIL`), emit `MS_OP_DEFINE_GLOBAL` with name constant index
- `MS_EXPR_VARIABLE`: emit `MS_OP_GET_GLOBAL` with name constant index
- `MS_EXPR_ASSIGN`: compile value, emit `MS_OP_SET_GLOBAL` with name constant index
- Undefined variable check: during compilation, variables used before definition should produce a compile error (for locals in later cycle; for globals, this may be deferred to runtime)

**Verify GREEN**: `cmake --build build && ./build/test_compiler` — variable tests pass

**REFACTOR**: Consolidate identifier constant lookup (reuse existing constant if same string already in pool)

### Cycle 5: Local Variables and Scoping

**RED** — Write failing test:
- Add `test_compile_local_var()`: compile `"{ var x = 1\nprint x }"`, verify: `OP_CONSTANT(1)`, `OP_DEFINE_LOCAL`(slot 0), `OP_GET_LOCAL(0)`, `OP_PRINT`, `OP_POP` — no global ops
- Add `test_compile_scope_depth()`: compile `"{ var x = 1\n{ var y = 2\nprint x\nprint y }\nprint x }"`, verify local slot indices and scope behavior
- Add `test_compile_set_local()`: compile `"{ var x = 1\nx = 2 }"`, verify `OP_SET_LOCAL`
- Add `test_compile_error_read_own_init()`: compile `"var x = x"`, verify compile error (reading own uninitialized local)
- Expected failure: wrong opcodes (global instead of local) or missing scoping

**Verify RED**: `./build/test_compiler` — local variable tests fail

**GREEN** — Minimal implementation:
- `beginScope()`: increment `scopeDepth`
- `endScope()`: decrement `scopeDepth`, emit `MS_OP_POP` for each local variable that goes out of scope
- `declareVariable()`: add local to current state's locals array with current scopeDepth
- `defineVariable()`: mark local as initialized (set `depth = scopeDepth`)
- `resolveLocal()`: walk locals array backward to find matching name, return slot index (or -1 if not found / error if in own initializer)
- Modify variable compilation: if `resolveLocal()` finds the variable, use `OP_GET_LOCAL`/`OP_SET_LOCAL` with slot index instead of global ops
- Compile `MS_STMT_BLOCK`: `beginScope()`, compile each statement in block, `endScope()`

**Verify GREEN**: `cmake --build build && ./build/test_compiler` — local variable and scope tests pass

**REFACTOR**: Verify slot indices are correct across nested scopes

### Cycle 6: If/Else Statements

**RED** — Write failing test:
- Add `test_compile_if()`: compile `"if (true) print 1"`, verify: `OP_TRUE`, `OP_JUMP_IF_FALSE(then_jump)`, `OP_POP`, `OP_CONSTANT(1)`, `OP_PRINT`, `OP_JUMP(end_jump)`, patching then_jump to else section, `OP_POP`
- Add `test_compile_if_else()`: compile `"if (true) print 1 else print 2"`, verify both branches with correct jump offsets
- Add `test_compile_nested_if()`: compile nested if-else chain, verify all jumps patch correctly
- Expected failure: wrong jump offsets or missing jump instructions

**Verify RED**: `./build/test_compiler` — if/else tests fail with wrong bytecode

**GREEN** — Minimal implementation:
- Compile `MS_STMT_IF`:
  1. Compile condition expression
  2. Emit `MS_OP_JUMP_IF_FALSE` (then jump) — save offset to patch later
  3. Emit `MS_OP_POP` (pop condition)
  4. Compile then-branch
  5. Emit `MS_OP_JUMP` (else jump) — save offset to patch later
  6. Patch then-jump to here
  7. Emit `MS_OP_POP` (pop condition on false path)
  8. If else-branch exists, compile it
  9. Patch else-jump to here
- Use `emitJump()` returning offset, `patchJump()` to write destination

**Verify GREEN**: `cmake --build build && ./build/test_compiler` — if/else tests pass

**REFACTOR**: Verify jump offset arithmetic is consistent (2-byte signed offset)

### Cycle 7: While/For Loops with Break/Continue

**RED** — Write failing test:
- Add `test_compile_while()`: compile `"while (true) print 1"`, verify: loop start marker, `OP_TRUE`, `OP_JUMP_IF_FALSE(exit)`, `OP_POP`, `OP_CONSTANT(1)`, `OP_PRINT`, `OP_LOOP(loop_start)`, exit patch
- Add `test_compile_for()`: compile `"for (var i = 0; i < 10; i = i + 1) print i"`, verify: var decl, condition jump, body, increment, loop back, exit
- Add `test_compile_break()`: compile `"while (true) break"`, verify `OP_JUMP` to loop exit
- Add `test_compile_continue()`: compile `"while (true) continue"`, verify `OP_JUMP` to loop start (increment in for)
- Add `test_compile_nested_loops()`: verify break/continue target correct loop
- Expected failure: wrong loop offsets, break/continue not patched

**Verify RED**: `./build/test_compiler` — loop tests fail

**GREEN** — Minimal implementation:
- Compile `MS_STMT_WHILE`:
  1. Record `loopStart` offset
  2. Compile condition
  3. Emit `MS_OP_JUMP_IF_FALSE` (exit jump)
  4. Emit `MS_OP_POP`
  5. Compile body (with loop context for break/continue)
  6. Emit `MS_OP_LOOP` back to `loopStart`
  7. Patch exit jump
  8. Emit `MS_OP_POP` (for false branch)
- Compile `MS_STMT_FOR`:
  1. `beginScope()`
  2. Compile initializer (var decl or expr)
  3. Record `loopStart`
  4. Compile condition; emit exit jump if present, otherwise infinite loop
  5. Compile body
  6. Record increment offset; compile increment expression + `OP_POP`
  7. Emit `MS_OP_LOOP` back to `loopStart`
  8. Patch exit jump
  9. `endScope()`
- Break: emit `MS_OP_JUMP`, record in breakJumps array, patch all at loop exit
- Continue: emit `MS_OP_JUMP` to increment start (for) or loop start (while)
- `emitLoop()`: emit `MS_OP_LOOP` with offset back to given instruction

**Verify GREEN**: `cmake --build build && ./build/test_compiler` — all loop tests pass

**REFACTOR**: Factor out loop-context tracking (break/continue jump lists) into helper functions

### Cycle 8: Return and Expression Statements

**RED** — Write failing test:
- Add `test_compile_return_value()`: compile `"return 42"` (inside a function-like context), verify: `OP_CONSTANT(42)`, `OP_RETURN`
- Add `test_compile_return_nil()`: compile `"return"`, verify: `OP_NIL`, `OP_RETURN`
- Add `test_compile_return_at_top_level()`: compile `"return 1"` at top level, verify compile error
- Add `test_compile_expr_stmt()`: compile `"1 + 2"`, verify expression result is computed then `OP_POP`ped
- Add `test_compile_print()`: compile `"print 1 + 2"`, verify expression + `OP_PRINT`
- Expected failure: return not handled, print not recognized

**Verify RED**: `./build/test_compiler` — return/print tests fail

**GREEN** — Minimal implementation:
- Compile `MS_STMT_RETURN`: if in top-level (`MS_FUNC_SCRIPT`), report error; compile return value (or emit `OP_NIL`), emit `MS_OP_RETURN`
- Compile `MS_STMT_EXPR`: compile expression, emit `MS_OP_POP`
- Handle print: if statement starts with `print` keyword, compile expression, emit `MS_OP_PRINT` instead of `MS_OP_POP`
- Compile `MS_STMT_BREAK` / `MS_STMT_CONTINUE`: emit jumps as handled in Cycle 7; if not inside loop, report compile error

**Verify GREEN**: `cmake --build build && ./build/test_compiler` — all tests pass

**REFACTOR**: Add stubs for `compileFuncDecl`, `compileClassDecl`, `compileImportStmt` that report "not yet implemented" errors, so the compiler doesn't crash on unimplemented constructs

## Acceptance Criteria

- [ ] Compile `"print 1 + 2"` → bytecode with OP_CONSTANT, OP_ADD, OP_PRINT
- [ ] Compile `"var x = 42; print x"` → OP_CONSTANT, OP_DEFINE_GLOBAL, OP_GET_GLOBAL
- [ ] Compile `"if (true) print 1 else print 2"` → OP_TRUE, OP_JUMP_IF_FALSE, ...
- [ ] Compile `"while (true) print 1"` → OP_LOOP
- [ ] Compile `"{ var x = 1; print x }"` → local variable with scope depth
- [ ] Compile `"for (var i = 0; i < 10; i = i + 1) print i"` → for loop bytecode
- [ ] Compile `"break"` and `"continue"` → correct jump patching
- [ ] Error on undefined variable access
- [ ] Error on redeclaring variable in same scope

## Notes

- Internal static functions: `emitByte`, `emitBytes`, `emitLoop`, `emitJump`, `emitReturn`, `emitConstant`, `makeConstant`, `identifierConstant`, `declareVariable`, `defineVariable`, `markInitialized`, `resolveLocal`, `beginScope`, `endScope`, `compileExpr`, `compileStmt`, `compileBlock`, `compileVarDecl`, `compileIfStmt`, `compileWhileStmt`, `compileForStmt`, `compileReturnStmt`, `compileBreakStmt`, `compileContinueStmt`, `compileExprStmt`
- Statements NOT covered in this task (stubs that report error): `compileFuncDecl` (T16), `compileClassDecl` (T18), `compileImportStmt` (T20)
- `ms_compiler_mark_roots()` is needed for GC integration (marks compiler-allocated objects as reachable)
