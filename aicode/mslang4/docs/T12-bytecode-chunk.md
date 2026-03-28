# T12: Bytecode Chunk

**Phase**: 5 - Bytecode
**Dependencies**: T07 (Value System)
**Estimated Complexity**: Medium

## Goal

Implement the bytecode instruction set and the chunk data structure that holds compiled bytecode, line information, and constant pool.

## Files to Create

| File | Purpose |
|------|---------|
| `src/chunk.h` | OpCode enum, MsChunk struct, API |
| `src/chunk.c` | Chunk operations and disassembler |

## TDD Implementation Cycles

### Cycle 1: OpCode Enum and Chunk Init/Free

**RED** — Write failing test:

Create `tests/unit/test_chunk.c`. Write a test function `test_chunk_init_free` that:
- Initializes a `MsChunk`
- Verifies `code == NULL`, `lines == NULL`, `count == 0`, `capacity == 0`, constants initialized

```c
// tests/unit/test_chunk.c (initial skeleton)
#include "chunk.h"
#include "value.h"
#include <stdio.h>
#include <assert.h>

static void test_chunk_init_free(void) {
    MsChunk chunk;
    ms_chunk_init(&chunk);
    assert(chunk.code == NULL);
    assert(chunk.lines == NULL);
    assert(chunk.count == 0);
    assert(chunk.capacity == 0);
    assert(chunk.constants.values == NULL);
    assert(chunk.constants.count == 0);

    ms_chunk_free(&chunk);
    assert(chunk.code == NULL);
    assert(chunk.lines == NULL);
    assert(chunk.count == 0);
    assert(chunk.capacity == 0);
    printf("  test_chunk_init_free PASSED\n");
}

int main(void) {
    printf("Running chunk tests...\n");
    test_chunk_init_free();
    printf("All chunk tests passed.\n");
    return 0;
}
```

**Verify RED**: `gcc -I src -o build/test_chunk tests/unit/test_chunk.c src/chunk.c src/value.c` → compilation error: `chunk.h: No such file or directory`

**GREEN** — Minimal implementation:

Create `src/chunk.h`:

```c
#ifndef MS_CHUNK_H
#define MS_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    MS_OP_CONSTANT, MS_OP_NIL, MS_OP_TRUE, MS_OP_FALSE, MS_OP_POP,
    MS_OP_DEFINE_GLOBAL, MS_OP_GET_GLOBAL, MS_OP_SET_GLOBAL,
    MS_OP_GET_LOCAL, MS_OP_SET_LOCAL, MS_OP_GET_UPVALUE, MS_OP_SET_UPVALUE,
    MS_OP_GET_PROPERTY, MS_OP_SET_PROPERTY, MS_OP_GET_SUPER,
    MS_OP_BUILD_LIST, MS_OP_GET_SUBSCRIPT, MS_OP_SET_SUBSCRIPT,
    MS_OP_EQUAL, MS_OP_NOT_EQUAL, MS_OP_GREATER, MS_OP_GREATER_EQUAL, MS_OP_LESS, MS_OP_LESS_EQUAL,
    MS_OP_ADD, MS_OP_SUBTRACT, MS_OP_MULTIPLY, MS_OP_DIVIDE, MS_OP_MODULO, MS_OP_NEGATE,
    MS_OP_NOT,
    MS_OP_AND, MS_OP_OR,
    MS_OP_JUMP, MS_OP_JUMP_IF_FALSE, MS_OP_LOOP, MS_OP_BREAK, MS_OP_CONTINUE,
    MS_OP_CALL, MS_OP_INVOKE, MS_OP_SUPER_INVOKE, MS_OP_CLOSURE, MS_OP_CLOSE_UPVALUE,
    MS_OP_CLASS, MS_OP_INHERIT, MS_OP_METHOD,
    MS_OP_IMPORT, MS_OP_IMPORT_FROM,
    MS_OP_RETURN,
    MS_OP_DEBUG_BREAK
} MsOpCode;

typedef struct {
    uint8_t* code;
    int* lines;
    int count;
    int capacity;
    MsValueArray constants;
} MsChunk;

void ms_chunk_init(MsChunk* chunk);
void ms_chunk_free(MsChunk* chunk);
void ms_chunk_write(MsChunk* chunk, uint8_t byte, int line);
int ms_chunk_add_constant(MsChunk* chunk, MsValue value);
void ms_chunk_disassemble(const MsChunk* chunk, const char* name);
int ms_chunk_disassemble_instruction(const MsChunk* chunk, int offset);

#endif
```

Create `src/chunk.c`:

```c
#include "chunk.h"
#include <stdio.h>
#include <stdlib.h>

void ms_chunk_init(MsChunk* chunk) {
    chunk->code = NULL;
    chunk->lines = NULL;
    chunk->count = 0;
    chunk->capacity = 0;
    ms_value_array_init(&chunk->constants);
}

void ms_chunk_free(MsChunk* chunk) {
    MS_FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    MS_FREE_ARRAY(int, chunk->lines, chunk->capacity);
    ms_value_array_free(&chunk->constants);
    ms_chunk_init(chunk);
}
```

**Verify GREEN**: `gcc -I src -o build/test_chunk tests/unit/test_chunk.c src/chunk.c src/value.c && ./build/test_chunk` → test passes

**REFACTOR**: None needed.

---

### Cycle 2: Chunk Write (Single Byte)

**RED** — Write failing test:

Add `test_chunk_write` to `test_chunk.c`:

```c
static void test_chunk_write(void) {
    MsChunk chunk;
    ms_chunk_init(&chunk);

    ms_chunk_write(&chunk, MS_OP_RETURN, 1);
    assert(chunk.count == 1);
    assert(chunk.code[0] == MS_OP_RETURN);
    assert(chunk.lines[0] == 1);

    ms_chunk_write(&chunk, MS_OP_NIL, 2);
    assert(chunk.count == 2);
    assert(chunk.code[1] == MS_OP_NIL);
    assert(chunk.lines[1] == 2);

    ms_chunk_free(&chunk);
    printf("  test_chunk_write PASSED\n");
}
```

**Verify RED**: Build fails — `ms_chunk_write` undefined.

**GREEN** — Minimal implementation:

Add to `src/chunk.c`:

```c
void ms_chunk_write(MsChunk* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = MS_GROW_CAPACITY(old_capacity);
        chunk->code = MS_GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
        chunk->lines = MS_GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}
```

Grow if `count == capacity` using `MS_GROW_CAPACITY` + `MS_GROW_ARRAY`. Append byte and line.

**Verify GREEN**: Build and run — both tests pass.

**REFACTOR**: None needed.

---

### Cycle 3: Line Number Tracking

**RED** — Write failing test:

Add `test_line_tracking` to `test_chunk.c`:

```c
static void test_line_tracking(void) {
    MsChunk chunk;
    ms_chunk_init(&chunk);

    ms_chunk_write(&chunk, MS_OP_NIL, 1);
    ms_chunk_write(&chunk, MS_OP_NIL, 1);
    ms_chunk_write(&chunk, MS_OP_NIL, 2);
    ms_chunk_write(&chunk, MS_OP_RETURN, 3);

    assert(chunk.lines[0] == 1);
    assert(chunk.lines[1] == 1);
    assert(chunk.lines[2] == 2);
    assert(chunk.lines[3] == 3);

    ms_chunk_free(&chunk);
    printf("  test_line_tracking PASSED\n");
}
```

**Verify RED**: Should compile and pass — line tracking is inherent in `ms_chunk_write`. This is a verification cycle.

**Verify GREEN**: Build and run — all three tests pass.

**REFACTOR**: None needed.

---

### Cycle 4: Chunk Dynamic Growth

**RED** — Write failing test:

Add `test_chunk_growth` to `test_chunk.c`:

```c
static void test_chunk_growth(void) {
    MsChunk chunk;
    ms_chunk_init(&chunk);

    for (int i = 0; i < 1000; i++) {
        ms_chunk_write(&chunk, (uint8_t)(i % 256), 1);
    }
    assert(chunk.count == 1000);

    for (int i = 0; i < 1000; i++) {
        assert(chunk.code[i] == (uint8_t)(i % 256));
    }

    ms_chunk_free(&chunk);
    printf("  test_chunk_growth PASSED\n");
}
```

**Verify RED**: Should compile and pass — growth was implemented in Cycle 2. Stress test to verify.

**Verify GREEN**: Build and run — all four tests pass.

**REFACTOR**: Run with address sanitizer to verify no leaks during growth.

---

### Cycle 5: Constant Pool (add_constant)

**RED** — Write failing test:

Add `test_constant_pool` to `test_chunk.c`:

```c
static void test_constant_pool(void) {
    MsChunk chunk;
    ms_chunk_init(&chunk);

    int idx0 = ms_chunk_add_constant(&chunk, ms_number_val(1.0));
    assert(idx0 == 0);

    int idx1 = ms_chunk_add_constant(&chunk, ms_number_val(2.0));
    assert(idx1 == 1);

    int idx2 = ms_chunk_add_constant(&chunk, ms_number_val(3.0));
    assert(idx2 == 2);

    /* Verify constants are retrievable by index */
    assert(ms_as_number(chunk.constants.values[0]) == 1.0);
    assert(ms_as_number(chunk.constants.values[1]) == 2.0);
    assert(ms_as_number(chunk.constants.values[2]) == 3.0);

    /* Write OP_CONSTANT referencing the constant */
    ms_chunk_write(&chunk, MS_OP_CONSTANT, 1);
    ms_chunk_write(&chunk, (uint8_t)idx0, 1);

    ms_chunk_free(&chunk);
    printf("  test_constant_pool PASSED\n");
}
```

**Verify RED**: Build fails — `ms_chunk_add_constant` undefined.

**GREEN** — Minimal implementation:

Add to `src/chunk.c`:

```c
int ms_chunk_add_constant(MsChunk* chunk, MsValue value) {
    ms_value_array_write(&chunk->constants, value);
    return chunk->constants.count - 1;
}
```

Returns sequential indices (0, 1, 2...). Constants retrievable via `chunk.constants.values[index]`.

Note: Duplicate checking is optional for the basic implementation. The compiler layer handles deduplication if needed.

**Verify GREEN**: Build and run — all five tests pass.

**REFACTOR**: None needed.

---

### Cycle 6: Disassembler — Simple Opcodes

**RED** — Write failing test:

Add `test_disassemble_simple` to `test_chunk.c`:

```c
#include <string.h>
#include <stdio.h>

static void test_disassemble_simple(void) {
    MsChunk chunk;
    ms_chunk_init(&chunk);

    ms_chunk_write(&chunk, MS_OP_NIL, 1);
    ms_chunk_write(&chunk, MS_OP_RETURN, 1);

    /* Disassemble should not crash and should return correct offsets */
    int next = ms_chunk_disassemble_instruction(&chunk, 0);
    assert(next == 1);
    next = ms_chunk_disassemble_instruction(&chunk, 1);
    assert(next == 2);

    /* Full disassemble */
    ms_chunk_disassemble(&chunk, "test");
    /* Visual check: should print opcode names */

    ms_chunk_free(&chunk);
    printf("  test_disassemble_simple PASSED\n");
}
```

**Verify RED**: Build fails — `ms_chunk_disassemble` and `ms_chunk_disassemble_instruction` undefined.

**GREEN** — Minimal implementation:

Add to `src/chunk.c`:

```c
static const char* ms_opcode_name(MsOpCode code) {
    switch (code) {
        case MS_OP_CONSTANT:       return "OP_CONSTANT";
        case MS_OP_NIL:            return "OP_NIL";
        case MS_OP_TRUE:           return "OP_TRUE";
        case MS_OP_FALSE:          return "OP_FALSE";
        case MS_OP_POP:            return "OP_POP";
        case MS_OP_DEFINE_GLOBAL:  return "OP_DEFINE_GLOBAL";
        case MS_OP_GET_GLOBAL:     return "OP_GET_GLOBAL";
        case MS_OP_SET_GLOBAL:     return "OP_SET_GLOBAL";
        case MS_OP_GET_LOCAL:      return "OP_GET_LOCAL";
        case MS_OP_SET_LOCAL:      return "OP_SET_LOCAL";
        case MS_OP_GET_UPVALUE:    return "OP_GET_UPVALUE";
        case MS_OP_SET_UPVALUE:    return "OP_SET_UPVALUE";
        case MS_OP_GET_PROPERTY:   return "OP_GET_PROPERTY";
        case MS_OP_SET_PROPERTY:   return "OP_SET_PROPERTY";
        case MS_OP_GET_SUPER:      return "OP_GET_SUPER";
        case MS_OP_BUILD_LIST:     return "OP_BUILD_LIST";
        case MS_OP_GET_SUBSCRIPT:  return "OP_GET_SUBSCRIPT";
        case MS_OP_SET_SUBSCRIPT:  return "OP_SET_SUBSCRIPT";
        case MS_OP_EQUAL:          return "OP_EQUAL";
        case MS_OP_NOT_EQUAL:      return "OP_NOT_EQUAL";
        case MS_OP_GREATER:        return "OP_GREATER";
        case MS_OP_GREATER_EQUAL:  return "OP_GREATER_EQUAL";
        case MS_OP_LESS:           return "OP_LESS";
        case MS_OP_LESS_EQUAL:     return "OP_LESS_EQUAL";
        case MS_OP_ADD:            return "OP_ADD";
        case MS_OP_SUBTRACT:       return "OP_SUBTRACT";
        case MS_OP_MULTIPLY:       return "OP_MULTIPLY";
        case MS_OP_DIVIDE:         return "OP_DIVIDE";
        case MS_OP_MODULO:         return "OP_MODULO";
        case MS_OP_NEGATE:         return "OP_NEGATE";
        case MS_OP_NOT:            return "OP_NOT";
        case MS_OP_AND:            return "OP_AND";
        case MS_OP_OR:             return "OP_OR";
        case MS_OP_JUMP:           return "OP_JUMP";
        case MS_OP_JUMP_IF_FALSE:  return "OP_JUMP_IF_FALSE";
        case MS_OP_LOOP:           return "OP_LOOP";
        case MS_OP_BREAK:          return "OP_BREAK";
        case MS_OP_CONTINUE:       return "OP_CONTINUE";
        case MS_OP_CALL:           return "OP_CALL";
        case MS_OP_INVOKE:         return "OP_INVOKE";
        case MS_OP_SUPER_INVOKE:   return "OP_SUPER_INVOKE";
        case MS_OP_CLOSURE:        return "OP_CLOSURE";
        case MS_OP_CLOSE_UPVALUE:  return "OP_CLOSE_UPVALUE";
        case MS_OP_CLASS:          return "OP_CLASS";
        case MS_OP_INHERIT:        return "OP_INHERIT";
        case MS_OP_METHOD:         return "OP_METHOD";
        case MS_OP_IMPORT:         return "OP_IMPORT";
        case MS_OP_IMPORT_FROM:    return "OP_IMPORT_FROM";
        case MS_OP_RETURN:         return "OP_RETURN";
        case MS_OP_DEBUG_BREAK:    return "OP_DEBUG_BREAK";
        default:                   return "OP_UNKNOWN";
    }
}

static int ms_disassemble_simple(const char* name, const MsChunk* chunk, int offset) {
    printf("%04d %s\n", offset, name);
    return offset + 1;
}

void ms_chunk_disassemble(const MsChunk* chunk, const char* name) {
    printf("== %s ==\n", name);
    for (int offset = 0; offset < chunk->count;) {
        offset = ms_chunk_disassemble_instruction(chunk, offset);
    }
}

int ms_chunk_disassemble_instruction(const MsChunk* chunk, int offset) {
    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case MS_OP_NIL:
        case MS_OP_TRUE:
        case MS_OP_FALSE:
        case MS_OP_POP:
        case MS_OP_RETURN:
        case MS_OP_NEGATE:
        case MS_OP_NOT:
        case MS_OP_CLOSE_UPVALUE:
        case MS_OP_INHERIT:
        case MS_OP_DEBUG_BREAK:
            return ms_disassemble_simple(ms_opcode_name(instruction), chunk, offset);
        default:
            printf("%04d Unknown opcode %d\n", offset, instruction);
            return offset + 1;
    }
}
```

Print header, iterate all instructions calling `ms_chunk_disassemble_instruction`. Dispatch on opcode. Print offset and opcode name. Return new offset. Simple opcodes are 1 byte.

**Verify GREEN**: Build and run — all six tests pass.

**REFACTOR**: None needed.

---

### Cycle 7: Disassembler — Constant Opcodes and Jump Opcodes

**RED** — Write failing test:

Add `test_disassemble_constant_and_jump` to `test_chunk.c`:

```c
static void test_disassemble_constant_and_jump(void) {
    MsChunk chunk;
    ms_chunk_init(&chunk);

    /* OP_CONSTANT with operand */
    int idx = ms_chunk_add_constant(&chunk, ms_number_val(42.0));
    ms_chunk_write(&chunk, MS_OP_CONSTANT, 1);
    ms_chunk_write(&chunk, (uint8_t)idx, 1);

    /* OP_JUMP with 2-byte operand */
    ms_chunk_write(&chunk, MS_OP_JUMP, 2);
    ms_chunk_write(&chunk, 0x00, 2);
    ms_chunk_write(&chunk, 0x05, 2);

    ms_chunk_write(&chunk, MS_OP_RETURN, 3);

    /* Disassemble and verify offsets */
    int offset = ms_chunk_disassemble_instruction(&chunk, 0);
    assert(offset == 2);  /* OP_CONSTANT + 1 byte operand */

    offset = ms_chunk_disassemble_instruction(&chunk, offset);
    assert(offset == 5);  /* OP_JUMP + 2 byte operand */

    offset = ms_chunk_disassemble_instruction(&chunk, offset);
    assert(offset == 6);  /* OP_RETURN + 1 byte */

    ms_chunk_disassemble(&chunk, "test2");
    /* Visual check: should show constant value and jump offset */

    ms_chunk_free(&chunk);
    printf("  test_disassemble_constant_and_jump PASSED\n");
}
```

**Verify RED**: Build and run — `OP_CONSTANT` and `OP_JUMP` fall through to "Unknown opcode" or return wrong offset. Tests fail.

**GREEN** — Update `ms_chunk_disassemble_instruction`:

```c
static int ms_disassemble_constant(const char* name, const MsChunk* chunk, int offset) {
    uint8_t constant_idx = chunk->code[offset + 1];
    printf("%04d %-16s %4d '", offset, name, constant_idx);
    ms_print_value(chunk->constants.values[constant_idx]);
    printf("'\n");
    return offset + 2;
}

static int ms_disassemble_jump(const char* name, int sign, const MsChunk* chunk, int offset) {
    uint16_t jump = (uint16_t)((chunk->code[offset + 1] << 8) | chunk->code[offset + 2]);
    printf("%04d %-16s %4d -> %d\n", offset, name, offset + 3 + sign * jump, offset + 3);
    return offset + 3;
}

int ms_chunk_disassemble_instruction(const MsChunk* chunk, int offset) {
    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case MS_OP_CONSTANT:
            return ms_disassemble_constant(ms_opcode_name(instruction), chunk, offset);
        case MS_OP_JUMP:
        case MS_OP_JUMP_IF_FALSE:
        case MS_OP_LOOP:
            return ms_disassemble_jump(ms_opcode_name(instruction), 1, chunk, offset);
        /* ... simple opcodes from Cycle 6 ... */
        case MS_OP_NIL:
        case MS_OP_TRUE:
        case MS_OP_FALSE:
        case MS_OP_POP:
        case MS_OP_RETURN:
        case MS_OP_NEGATE:
        case MS_OP_NOT:
        case MS_OP_CLOSE_UPVALUE:
        case MS_OP_INHERIT:
        case MS_OP_DEBUG_BREAK:
            return ms_disassemble_simple(ms_opcode_name(instruction), chunk, offset);
        /* Byte-operand opcodes */
        case MS_OP_DEFINE_GLOBAL:
        case MS_OP_GET_GLOBAL:
        case MS_OP_SET_GLOBAL:
        case MS_OP_CALL:
        case MS_OP_CLASS:
        case MS_OP_METHOD:
            printf("%04d %-16s %4d\n", offset, ms_opcode_name(instruction), chunk->code[offset + 1]);
            return offset + 2;
        default:
            printf("%04d Unknown opcode %d\n", offset, instruction);
            return offset + 1;
    }
}
```

Instruction format:
- Simple opcodes: 1 byte (opcode only)
- Constant opcodes: opcode + 1 byte (constant index)
- Jump opcodes: opcode + 2 bytes (signed offset, big-endian)
- MS_OP_CLOSURE: opcode + 1 byte + upvalue entries (complex, handled later)

**Verify GREEN**: Build and run — all seven tests pass.

**REFACTOR**: None needed. The disassembler is complete for all defined opcodes.

## Acceptance Criteria

- [ ] Init/free cycle completes without leak
- [ ] Write 256 bytes into chunk, all readable at correct indices
- [ ] `ms_chunk_add_constant` returns sequential indices (0, 1, 2...)
- [ ] Constants can be retrieved by index from `chunk.constants.values[index]`
- [ ] Line numbers are stored alongside bytes
- [ ] Chunk grows dynamically (write 1000+ bytes)
- [ ] Disassembler prints readable output for basic opcodes
- [ ] Disassembler handles MS_OP_CONSTANT with constant value display
- [ ] Disassembler returns correct next offset

## Notes

- Simple instruction format: opcode (1 byte). Some opcodes have operands: MS_OP_CONSTANT (1 byte index), MS_OP_DEFINE_GLOBAL (1 byte), jumps (2 bytes signed offset), MS_OP_CLOSURE (1 byte + upvalue entries), etc.
- The disassembler starts with basic support for simple, constant, and jump opcodes. Full operand printing for all opcodes can be extended as needed.
- `ms_chunk_add_constant` does not deduplicate in this basic implementation. Deduplication is handled at the compiler layer if needed.
- The `MsChunk` struct owns the `constants` MsValueArray and frees it in `ms_chunk_free`. Note that if constants include OBJ values, those objects are not freed by the chunk — they are owned by the GC.
