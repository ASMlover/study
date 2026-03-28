# T15: VM Core

**Phase**: 6 - Compiler & VM (Basic)
**Dependencies**: T14 (Compiler - Basic), T09 (Hash Table)
**Estimated Complexity**: High

## Goal

Implement the core virtual machine with a stack-based execution engine. This task covers basic arithmetic, variable access, control flow, and the `print` statement. Combined with T14, this creates a working end-to-end pipeline: source → scanner → parser → compiler → VM → output.

## Files to Create

| File | Purpose |
|------|---------|
| `src/vm.h` | VM struct, call frame, interpret API |
| `src/vm.c` | VM execution engine (basic ops) |

## TDD Implementation Cycles

### Cycle 1: VM Init/Free and Stack Operations

**RED** — Write failing test:
- Create `tests/unit/test_vm.c`
- Write `test_vm_init_free()`: call `ms_vm_init(&vm)`, verify `stackTop == stack`, `frameCount == 0`, `initialized == true`; then `ms_vm_free(&vm)`, verify no crash/leak
- Write `test_vm_push_pop()`: init VM, push a value, verify `stackTop == stack + 1`; pop, verify value matches, `stackTop == stack`
- Write `test_vm_peek()`: push two values, verify `peek(0)` returns top, `peek(1)` returns second
- Expected failure: linker error — `ms_vm_init`, `ms_vm_free` undefined

**Verify RED**: `cmake --build build 2>&1 | grep "undefined reference"` — link errors for VM symbols

**GREEN** — Minimal implementation:
- Create `src/vm.h` with full struct definitions:
```c
#ifndef MS_VM_H
#define MS_VM_H

#include "chunk.h"
#include "table.h"
#include "object.h"
#include "compiler.h"

typedef struct { MsClosure* closure; uint8_t* ip; MsValue* slots; } MsCallFrame;

typedef struct MsVM {
    MsValue stack[MS_STACK_MAX];
    MsValue* stackTop;
    MsCallFrame frames[MS_FRAMES_MAX];
    int frameCount;
    MsTable globals;
    MsTable strings;
    MsTable modules;
    char** modulePaths; int modulePathCount; int modulePathCapacity;
    MsObject* objects;
    size_t bytesAllocated;
    size_t nextGC;
    MsObject** grayStack; int grayCount; int grayCapacity;
    MsCompiler* compiler;
    bool initialized;
} MsVM;

typedef enum { MS_INTERPRET_OK, MS_INTERPRET_COMPILE_ERROR, MS_INTERPRET_RUNTIME_ERROR } MsInterpretResult;

void ms_vm_init(MsVM* vm);
void ms_vm_free(MsVM* vm);
MsInterpretResult ms_vm_interpret(MsVM* vm, const char* source);

#endif
```
- Create `src/vm.c` implementing:
  - `ms_vm_init()`: `stackTop = stack`, `frameCount = 0`, `ms_table_init` for globals/strings/modules, `objects = NULL`, `bytesAllocated = 0`, `nextGC = 1024 * 1024`, `grayStack = NULL`, `grayCount = 0`, `grayCapacity = 0`, `compiler = NULL`, `modulePaths = NULL`, `initialized = true`
  - `ms_vm_free()`: free all objects via `ms_object_free()`, free gray stack, free module paths, `ms_table_free` for globals/strings/modules, reset stack
  - Internal static: `push()`, `pop()`, `peek()`, `resetStack()`, `runtimeError()`
- Update `src/object.h`/`src/object.c` to integrate with VM for allocation: `ms_alloc_object()` links into `vm->objects` list

**Verify GREEN**: `cmake --build build && ./build/test_vm` — init/free and stack tests pass

**REFACTOR**: Ensure `resetStack()` properly reinitializes all stack-related state

### Cycle 2: Compile and Interpret Literals

**RED** — Write failing test:
- Add `test_interpret_number()`: `ms_vm_interpret(vm, "42")` returns `MS_INTERPRET_OK`
- Add `test_interpret_bool_nil()`: `ms_vm_interpret(vm, "true")`, `ms_vm_interpret(vm, "nil")` all return `MS_INTERPRET_OK`
- Add `test_interpret_empty()`: `ms_vm_interpret(vm, "")` returns `MS_INTERPRET_OK`
- Expected failure: `ms_vm_interpret` not implemented, returns wrong result or crashes

**Verify RED**: `./build/test_vm` — interpret tests fail

**GREEN** — Minimal implementation:
- Implement `ms_vm_interpret()`:
  1. Call `ms_compiler_compile()` to compile source
  2. If compile error (NULL return), return `MS_INTERPRET_COMPILE_ERROR`
  3. Create `MsClosure` wrapping the compiled function
  4. Set up first call frame: `frame->closure = closure`, `frame->ip = function->chunk.code`, `frame->slots = vm.stack`
  5. Call `run()` static function
  6. Return `run()` result
- Implement `run()` — switch-based dispatch loop:
  - Read `*ip++`, switch on opcode
  - `MS_OP_CONSTANT`: read index, push `chunk->constants.values[index]`
  - `MS_OP_NIL`: push `MS_VALUE_NIL`
  - `MS_OP_TRUE`: push `MS_VALUE_BOOL(true)`
  - `MS_OP_FALSE`: push `MS_VALUE_BOOL(false)`
  - `MS_OP_POP`: `pop()`
  - `MS_OP_RETURN`: return `MS_INTERPRET_OK` (for top-level, just end execution)

**Verify GREEN**: `cmake --build build && ./build/test_vm` — literal interpretation tests pass

**REFACTOR**: Use computed goto (`dispatch_table`) or keep switch — defer optimization

### Cycle 3: Arithmetic and Comparison Operations

**RED** — Write failing test:
- Add `test_interpret_add()`: `ms_vm_interpret(vm, "print 1 + 2")` captures output "3"
- Add `test_interpret_subtract()`: `print 5 - 3` → "2"
- Add `test_interpret_multiply()`: `print 3 * 4` → "12"
- Add `test_interpret_divide()`: `print 10 / 2` → "5"
- Add `test_interpret_modulo()`: `print 10 % 3` → "1"
- Add `test_interpret_negate()`: `print -5` → "-5"
- Add `test_interpret_not()`: `print !true` → "false"
- Add `test_interpret_comparison()`: `print 1 < 2` → "true", `print 1 > 2` → "false", `print 1 == 1` → "true", `print 1 != 2` → "true"
- Add `test_interpret_type_error()`: `print 1 + "hello"` → runtime error
- Expected failure: opcodes not handled, output capture fails

**Verify RED**: `./build/test_vm` — arithmetic/comparison tests fail

**GREEN** — Minimal implementation:
- Extend `run()` dispatch:
  - `MS_OP_ADD`: pop two values; if both numbers → push result; if both strings → concatenate; else runtime error
  - `MS_OP_SUBTRACT`, `MS_OP_MULTIPLY`, `MS_OP_DIVIDE`, `MS_OP_MODULO`: pop two numbers, compute, push result; type mismatch → runtime error
  - `MS_OP_NEGATE`: pop number, negate, push; non-number → runtime error
  - `MS_OP_NOT`: pop, push boolean inverse
  - `MS_OP_EQUAL`: pop two, push `ms_value_equal(a, b)` result
  - `MS_OP_NOT_EQUAL`: like EQUAL then NOT
  - `MS_OP_GREATER`, `MS_OP_LESS`, `MS_OP_GREATER_EQUAL`, `MS_OP_LESS_EQUAL`: pop two numbers, compare, push bool
- String concatenation: allocate new string via `ms_string_copy()`, push as `MS_VALUE_OBJECT`
- Implement output capture for tests: use a callback or redirected stdout

**Verify GREEN**: `cmake --build build && ./build/test_vm` — arithmetic and comparison tests pass

**REFACTOR**: Consolidate binary op dispatch using a helper macro or function to reduce repetition

### Cycle 4: Variables (Global and Local)

**RED** — Write failing test:
- Add `test_interpret_global_var()`: `ms_vm_interpret(vm, "var x = 42\nprint x")` outputs "42"
- Add `test_interpret_set_global()`: `"var x = 1\nx = 2\nprint x"` → "2"
- Add `test_interpret_local_var()`: `"{ var x = 42\nprint x }"` → "42"
- Add `test_interpret_scope()`: `"var x = 1\n{ var x = 2\nprint x }\nprint x"` → "2" then "1"
- Add `test_interpret_undefined_var()`: `"print undefined"` → `MS_INTERPRET_RUNTIME_ERROR`
- Add `test_interpret_set_undefined_var()`: `"undefined = 1"` → runtime error or define implicitly (depending on design)
- Expected failure: global/local opcodes not handled

**Verify RED**: `./build/test_vm` — variable tests fail

**GREEN** — Minimal implementation:
- Extend `run()` dispatch:
  - `MS_OP_DEFINE_GLOBAL`: read name constant, pop value, `ms_table_set(&vm->globals, name, value)`
  - `MS_OP_GET_GLOBAL`: read name constant, `ms_table_get(&vm->globals, name, &value)`; if not found → runtime error "undefined variable"
  - `MS_OP_SET_GLOBAL`: read name constant, `ms_table_set(&vm->globals, name, value)`; pop and push value
  - `MS_OP_GET_LOCAL`: read slot index, push `frame->slots[slot]`
  - `MS_OP_SET_LOCAL`: read slot index, pop value, store at `frame->slots[slot]`
- Ensure call frame `slots` pointer is set correctly to base of stack for top-level script

**Verify GREEN**: `cmake --build build && ./build/test_vm` — variable tests pass

**REFACTOR**: Verify scope push/pop correctly manages stack slots

### Cycle 5: Control Flow (If/Else, While, For, Break/Continue)

**RED** — Write failing test:
- Add `test_interpret_if_true()`: `"if (true) print 1 else print 2"` → "1"
- Add `test_interpret_if_false()`: `"if (false) print 1 else print 2"` → "2"
- Add `test_interpret_while()`: `"var i = 0\nwhile (i < 5) { print i\ni = i + 1 }"` → outputs "0", "1", "2", "3", "4"
- Add `test_interpret_for()`: `"for (var i = 0; i < 3; i = i + 1) print i"` → "0", "1", "2"
- Add `test_interpret_break()`: `"var i = 0\nwhile (true) { print i\ni = i + 1\nif (i >= 3) break }"` → "0", "1", "2"
- Add `test_interpret_continue()`: `"for (var i = 0; i < 5; i = i + 1) { if (i == 2) continue\nprint i }"` → "0", "1", "3", "4"
- Add `test_interpret_nested_loops()`: nested while/for with break/continue targeting correct loop
- Expected failure: jump opcodes not handled

**Verify RED**: `./build/test_vm` — control flow tests fail

**GREEN** — Minimal implementation:
- Extend `run()` dispatch:
  - `MS_OP_JUMP`: read 2-byte signed offset, advance `ip += offset`
  - `MS_OP_JUMP_IF_FALSE`: read offset, pop value; if falsy, `ip += offset`
  - `MS_OP_LOOP`: read 2-byte signed offset, `ip -= offset` (jump backward)
  - `MS_OP_AND`: read offset, if top is falsy, skip; otherwise pop and continue
  - `MS_OP_OR`: read offset, if top is truthy, skip; otherwise pop and continue
  - `MS_OP_BREAK`: same as `MS_OP_JUMP` (already compiled as a jump to loop exit)
  - `MS_OP_CONTINUE`: same as `MS_OP_JUMP` (already compiled as a jump to increment/start)
- Jump offset encoding: 2 bytes, big-endian (high byte first), signed 16-bit

**Verify GREEN**: `cmake --build build && ./build/test_vm` — all control flow tests pass

**REFACTOR**: Extract `readShort()` helper to read 2-byte offset from IP

### Cycle 6: String Operations and Interning

**RED** — Write failing test:
- Add `test_interpret_string_concat()`: `"print \"hello\" + \" \" + \"world\""` → "hello world"
- Add `test_interpret_string_var()`: `"var s = \"test\"\nprint s"` → "test"
- Add `test_interpret_string_comparison()`: `"print \"abc\" == \"abc\""` → "true", `"print \"abc\" != \"def\""` → "true"
- Add `test_string_interning()`: intern the same string twice, verify they are the same pointer (identity check)
- Expected failure: string operations not working correctly

**Verify RED**: `./build/test_vm` — string tests fail

**GREEN** — Minimal implementation:
- Implement string interning: `ms_string_copy()` checks `vm->strings` table; if identical string exists, return it; otherwise allocate new, add to table
- String concatenation in `MS_OP_ADD`: compute total length, allocate new buffer, copy both strings, create interned result
- String equality: interned strings can be compared by pointer; non-interned fall back to `strcmp`
- `ms_value_print()`: format strings with quotes for debug, without for print output

**Verify GREEN**: `cmake --build build && ./build/test_vm` — string tests pass

**REFACTOR**: Ensure all string allocations go through interning path

### Cycle 7: Print Statement, Runtime Errors, and End-to-End Pipeline

**RED** — Write failing test:
- Add `test_interpret_print()`: `"print 42"` → "42\n"
- Add `test_interpret_print_string()`: `"print \"hello\""` → "hello\n"
- Add `test_interpret_print_bool()`: `"print true"` → "true\n"
- Add `test_interpret_runtime_error_stack_overflow()`: deeply recursive program → `MS_INTERPRET_RUNTIME_ERROR`
- Add `test_interpret_multiple_scripts()`: init VM, run script 1, run script 2, free VM — both succeed independently
- Add `test_interpret_full_program()`: `"var sum = 0\nfor (var i = 1; i <= 10; i = i + 1) sum = sum + i\nprint sum"` → "55"
- Write integration tests:
  - `tests/basic/arithmetic.ms`: comprehensive arithmetic
  - `tests/basic/variables.ms`: variable lifecycle
  - `tests/basic/control_flow.ms`: if/while/for/break/continue
  - `tests/basic/strings.ms`: string operations
- Expected failure: print not working, runtime errors not clean

**Verify RED**: `./build/test_vm` — end-to-end tests fail

**GREEN** — Minimal implementation:
- Extend `run()` dispatch:
  - `MS_OP_PRINT`: pop value, call `ms_value_print()`, output newline
- `runtimeError()`: format error message, print with line number, `resetStack()`, set `frameCount` back
- Stack overflow detection: in `call()`, check `frameCount >= MS_FRAMES_MAX` before creating new frame
- Ensure VM state is clean between `ms_vm_interpret()` calls (reset stack, don't reset globals)
- Run all integration test scripts: read `.ms` file, interpret, compare output

**Verify GREEN**: `cmake --build build && ./build/test_vm` — all tests pass including integration scripts

**REFACTOR**: Clean up output formatting, ensure error messages include source line info

## Acceptance Criteria

- [ ] `ms_vm_interpret(vm, "print 1 + 2")` outputs "3"
- [ ] `ms_vm_interpret(vm, "var x = 42\nprint x")` outputs "42"
- [ ] `ms_vm_interpret(vm, "if (true) print 1 else print 2")` outputs "1"
- [ ] `ms_vm_interpret(vm, "var i = 0\nwhile (i < 5) { print i\ni = i + 1 }")` outputs 0,1,2,3,4
- [ ] `ms_vm_interpret(vm, "for (var i = 0; i < 3; i = i + 1) print i")` outputs 0,1,2
- [ ] Undefined variable → runtime error
- [ ] Stack overflow → runtime error
- [ ] VM can be init'd, interpret multiple scripts, and free'd without leaks
- [ ] String concatenation: `print "hello" + " " + "world"` → "hello world"

## Notes

- Internal static helpers: `push()`, `pop()`, `peek()`, `resetStack()`, `runtimeError()`
- `ms_vm_interpret()` flow: compile → set up call frame → `run()` → return result
- This task also updates `object.h/object.c` to integrate with VM for allocation (`ms_alloc_object` links into `vm->objects` list) and implements string interning via `vm->strings` table
- GC fields (`bytesAllocated`, `nextGC`, `grayStack`, etc.) are initialized here but actual GC logic is implemented in T17
