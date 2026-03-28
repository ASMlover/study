# T07: Value System

**Phase**: 2 - Core Data Types
**Dependencies**: T02 (Common Definitions), T03 (Memory Subsystem)
**Estimated Complexity**: Medium

## Goal

Implement the tagged-union value representation that is the fundamental data type throughout the VM. Values are 16 bytes: a type tag plus a union of bool/double/object pointer.

## Files to Create

| File | Purpose |
|------|---------|
| `src/value.h` | Value type definitions, inline helpers, MsValueArray |
| `src/value.c` | Value comparison, printing, array operations |

## TDD Implementation Cycles

### Cycle 1: Value Constructors and Type Checkers

**RED** — Write failing test:

Create `tests/unit/test_value.c`. Write a test function `test_value_constructors` that:
- Calls `ms_nil_val()`, verifies `.type == MS_VAL_NIL`
- Calls `ms_bool_val(true)`, verifies `.type == MS_VAL_BOOL` and `.boolean == true`
- Calls `ms_bool_val(false)`, verifies `.boolean == false`
- Calls `ms_number_val(3.14)`, verifies `.type == MS_VAL_NUMBER` and `.number` is approximately 3.14
- Calls `ms_is_nil`, `ms_is_bool`, `ms_is_number`, `ms_is_obj` on appropriate values, verifies each returns true/false correctly
- Verifies `sizeof(MsValue) == 16` on 64-bit

```c
// tests/unit/test_value.c (initial skeleton)
#include "value.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

static void test_value_constructors(void) {
    assert(ms_is_nil(ms_nil_val()));
    assert(ms_nil_val().type == MS_VAL_NIL);

    MsValue bv = ms_bool_val(true);
    assert(ms_is_bool(bv));
    assert(ms_as_bool(bv) == true);

    MsValue nv = ms_number_val(3.14);
    assert(ms_is_number(nv));
    assert(fabs(ms_as_number(nv) - 3.14) < 1e-10);

    assert(sizeof(MsValue) == 16);
    printf("  test_value_constructors PASSED\n");
}

int main(void) {
    printf("Running value tests...\n");
    test_value_constructors();
    printf("All value tests passed.\n");
    return 0;
}
```

This will fail to compile because `value.h` does not exist.

**Verify RED**: `gcc -I src -o build/test_value tests/unit/test_value.c src/value.c` → compilation error: `value.h: No such file or directory`

**GREEN** — Minimal implementation:

Create `src/value.h` with the core type definitions and inline helpers:

```c
#ifndef MS_VALUE_H
#define MS_VALUE_H

#include "common.h"

typedef struct MsObject MsObject;

typedef enum { MS_VAL_NIL, MS_VAL_BOOL, MS_VAL_NUMBER, MS_VAL_OBJ } MsValueType;

typedef struct {
    MsValueType type;
    union { bool boolean; double number; MsObject* obj; };
} MsValue;

static inline MsValue ms_nil_val(void)    { return (MsValue){ .type = MS_VAL_NIL, .number = 0 }; }
static inline MsValue ms_bool_val(bool v) { return (MsValue){ .type = MS_VAL_BOOL, .boolean = v }; }
static inline MsValue ms_number_val(double v) { return (MsValue){ .type = MS_VAL_NUMBER, .number = v }; }
static inline MsValue ms_obj_val(MsObject* o) { return (MsValue){ .type = MS_VAL_OBJ, .obj = o }; }

static inline bool ms_is_nil(MsValue v)    { return v.type == MS_VAL_NIL; }
static inline bool ms_is_bool(MsValue v)   { return v.type == MS_VAL_BOOL; }
static inline bool ms_is_number(MsValue v) { return v.type == MS_VAL_NUMBER; }
static inline bool ms_is_obj(MsValue v)    { return v.type == MS_VAL_OBJ; }

static inline bool ms_as_bool(MsValue v)     { return v.boolean; }
static inline double ms_as_number(MsValue v) { return v.number; }
static inline MsObject* ms_as_obj(MsValue v) { return v.obj; }

#endif
```

Create `src/value.c` as an empty translation unit for now (just `#include "value.h"`). The inline functions in the header are sufficient for this cycle.

**Verify GREEN**: `gcc -I src -o build/test_value tests/unit/test_value.c src/value.c && ./build/test_value` → test passes

**REFACTOR**: None needed.

---

### Cycle 2: Value Equality

**RED** — Write failing test:

Add `test_value_equality` to `test_value.c`:

```c
static void test_value_equality(void) {
    assert(ms_values_equal(ms_number_val(42), ms_number_val(42)));
    assert(!ms_values_equal(ms_number_val(1), ms_number_val(2)));
    assert(ms_values_equal(ms_bool_val(true), ms_bool_val(true)));
    assert(!ms_values_equal(ms_bool_val(true), ms_bool_val(false)));
    assert(ms_values_equal(ms_nil_val(), ms_nil_val()));
    assert(!ms_values_equal(ms_nil_val(), ms_bool_val(false)));
    printf("  test_value_equality PASSED\n");
}
```

Call it from `main`. This will fail to link because `ms_values_equal` is not implemented.

**Verify RED**: `gcc -I src -o build/test_value tests/unit/test_value.c src/value.c` → linker error: undefined reference to `ms_values_equal`

**GREEN** — Minimal implementation:

Add declaration to `src/value.h`:
```c
bool ms_values_equal(MsValue a, MsValue b);
```

Implement in `src/value.c`:
```c
#include "value.h"
#include <stdbool.h>

bool ms_values_equal(MsValue a, MsValue b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case MS_VAL_NIL:    return true;
        case MS_VAL_BOOL:   return a.boolean == b.boolean;
        case MS_VAL_NUMBER: return a.number == b.number;
        case MS_VAL_OBJ:    return a.obj == b.obj;
    }
    return false;
}
```

Compare type tags first, then union payload. For OBJ type, use pointer equality (interned strings).

**Verify GREEN**: `gcc -I src -o build/test_value tests/unit/test_value.c src/value.c && ./build/test_value` → both tests pass

**REFACTOR**: None needed.

---

### Cycle 3: Falsey Semantics

**RED** — Write failing test:

Add `test_falsey` to `test_value.c`:

```c
static void test_falsey(void) {
    assert(ms_is_falsey(ms_nil_val()));
    assert(ms_is_falsey(ms_bool_val(false)));
    assert(!ms_is_falsey(ms_bool_val(true)));
    assert(!ms_is_falsey(ms_number_val(0)));
    assert(!ms_is_falsey(ms_number_val(1)));
    printf("  test_falsey PASSED\n");
}
```

**Verify RED**: Build fails — `ms_is_falsey` undeclared / undefined.

**GREEN** — Minimal implementation:

Add declaration to `src/value.h`:
```c
bool ms_is_falsey(MsValue value);
```

Implement in `src/value.c`:
```c
bool ms_is_falsey(MsValue value) {
    return ms_is_nil(value) || (ms_is_bool(value) && !ms_as_bool(value));
}
```

`nil` and `false` are falsey; everything else (including 0 and empty string) is truthy.

**Verify GREEN**: Build and run — all three tests pass.

**REFACTOR**: None needed.

---

### Cycle 4: Value Printing

**RED** — Write failing test:

Add `test_print_value` to `test_value.c`:

```c
#include <string.h>
#include <stdio.h>

static void test_print_value(void) {
    char buf[128];
    FILE* old = stdout;

    /* Test nil */
    stdout = fmemopen(buf, sizeof(buf), "w");
    ms_print_value(ms_nil_val());
    fflush(stdout);
    fclose(stdout);
    stdout = old;
    assert(strcmp(buf, "nil") == 0);

    /* Test bool true */
    stdout = fmemopen(buf, sizeof(buf), "w");
    ms_print_value(ms_bool_val(true));
    fflush(stdout);
    fclose(stdout);
    stdout = old;
    assert(strcmp(buf, "true") == 0);

    /* Test number */
    stdout = fmemopen(buf, sizeof(buf), "w");
    ms_print_value(ms_number_val(42));
    fflush(stdout);
    fclose(stdout);
    stdout = old;
    assert(strcmp(buf, "42") == 0);

    printf("  test_print_value PASSED\n");
}
```

Note: On Windows, `fmemopen` may not be available. Alternatively, use a portable approach with `freopen` to a temp file, or simply test that `ms_print_value` does not crash and returns. Adapt the test to your platform's capabilities. A simpler alternative:

```c
static void test_print_value(void) {
    printf("  Expect 'nil': "); ms_print_value(ms_nil_val()); printf("\n");
    printf("  Expect 'true': "); ms_print_value(ms_bool_val(true)); printf("\n");
    printf("  Expect 'false': "); ms_print_value(ms_bool_val(false)); printf("\n");
    printf("  Expect '42': "); ms_print_value(ms_number_val(42)); printf("\n");
    printf("  Expect '3.14': "); ms_print_value(ms_number_val(3.14)); printf("\n");
    printf("  test_print_value PASSED (visual check)\n");
}
```

**Verify RED**: Build fails — `ms_print_value` undeclared / undefined.

**GREEN** — Minimal implementation:

Add declaration to `src/value.h`:
```c
void ms_print_value(MsValue value);
```

Implement in `src/value.c`:
```c
#include <stdio.h>

void ms_print_value(MsValue value) {
    switch (value.type) {
        case MS_VAL_NIL:    printf("nil"); break;
        case MS_VAL_BOOL:   printf(value.boolean ? "true" : "false"); break;
        case MS_VAL_NUMBER: printf("%.14g", value.number); break;
        case MS_VAL_OBJ:    ms_object_print(value); break;
    }
}
```

Note: `ms_object_print` will be implemented in T08. For now, stub it or use a forward declaration. Since OBJ printing is tested in T08, this cycle only tests NIL, BOOL, NUMBER printing. Add a forward declaration in `value.h` or use a placeholder.

**Verify GREEN**: Build and run — test passes (NIL, BOOL, NUMBER output verified).

**REFACTOR**: None needed. The `ms_object_print` call will resolve when T08 is linked.

---

### Cycle 5: ValueArray Init/Free/Write

**RED** — Write failing test:

Add `test_value_array` to `test_value.c`:

```c
static void test_value_array_basic(void) {
    MsValueArray arr;
    ms_value_array_init(&arr);
    assert(arr.values == NULL);
    assert(arr.count == 0);
    assert(arr.capacity == 0);

    ms_value_array_write(&arr, ms_number_val(1));
    ms_value_array_write(&arr, ms_number_val(2));
    ms_value_array_write(&arr, ms_number_val(3));
    assert(arr.count == 3);
    assert(ms_as_number(arr.values[0]) == 1.0);
    assert(ms_as_number(arr.values[1]) == 2.0);
    assert(ms_as_number(arr.values[2]) == 3.0);

    ms_value_array_free(&arr);
    assert(arr.values == NULL);
    assert(arr.count == 0);
    assert(arr.capacity == 0);
    printf("  test_value_array_basic PASSED\n");
}
```

**Verify RED**: Build fails — `MsValueArray`, `ms_value_array_init`, etc. undeclared.

**GREEN** — Minimal implementation:

Add to `src/value.h`:
```c
typedef struct { MsValue* values; int count; int capacity; } MsValueArray;

void ms_value_array_init(MsValueArray* array);
void ms_value_array_free(MsValueArray* array);
void ms_value_array_write(MsValueArray* array, MsValue value);
```

Implement in `src/value.c`:
```c
void ms_value_array_init(MsValueArray* array) {
    array->values = NULL;
    array->count = 0;
    array->capacity = 0;
}

void ms_value_array_free(MsValueArray* array) {
    MS_FREE_ARRAY(MsValue, array->values, array->capacity);
    ms_value_array_init(array);
}

void ms_value_array_write(MsValueArray* array, MsValue value) {
    if (array->capacity < array->count + 1) {
        int old_capacity = array->capacity;
        array->capacity = MS_GROW_CAPACITY(old_capacity);
        array->values = MS_GROW_ARRAY(MsValue, array->values, old_capacity, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}
```

Grow with `MS_GROW_CAPACITY` + `MS_GROW_ARRAY` when `count == capacity`, append value.

**Verify GREEN**: Build and run — all four tests pass.

**REFACTOR**: None needed.

---

### Cycle 6: ValueArray Dynamic Growth

**RED** — Write failing test:

Add `test_value_array_growth` to `test_value.c`:

```c
static void test_value_array_growth(void) {
    MsValueArray arr;
    ms_value_array_init(&arr);

    for (int i = 0; i < 200; i++) {
        ms_value_array_write(&arr, ms_number_val(i));
    }
    assert(arr.count == 200);
    for (int i = 0; i < 200; i++) {
        assert(ms_as_number(arr.values[i]) == (double)i);
    }

    ms_value_array_free(&arr);
    printf("  test_value_array_growth PASSED\n");
}
```

**Verify RED**: This test should already compile and link (uses existing API). It will fail at runtime if growth is broken, but since Cycle 5 implemented growth, it should pass. Run to confirm.

**Verify GREEN**: `gcc -I src -o build/test_value tests/unit/test_value.c src/value.c && ./build/test_value` → all tests pass.

**REFACTOR**: Verify no memory leaks by running with a sanitizer if available:
`gcc -fsanitize=address -I src -o build/test_value tests/unit/test_value.c src/value.c && ./build/test_value`

## Acceptance Criteria

- [ ] `sizeof(MsValue) == 16` on 64-bit platforms
- [ ] `ms_nil_val().type == MS_VAL_NIL`
- [ ] `ms_bool_val(true).boolean == true`
- [ ] `ms_number_val(3.14).number` is approximately 3.14
- [ ] `ms_values_equal(ms_number_val(42), ms_number_val(42))` is true
- [ ] `ms_values_equal(ms_number_val(1), ms_number_val(2))` is false
- [ ] `ms_is_falsey(ms_nil_val())` is true
- [ ] `ms_is_falsey(ms_bool_val(false))` is true
- [ ] `ms_is_falsey(ms_number_val(0))` is false
- [ ] `ms_print_value(ms_number_val(42))` prints "42"
- [ ] MsValueArray init/write/free cycle works without leaks
- [ ] MsValueArray grows correctly (write 100+ values)

## Notes

- `ms_print_value` for OBJ type delegates to `ms_object_print()`, which is implemented in T08. Until T08 is linked, only NIL/BOOL/NUMBER printing can be tested.
- All inline helpers are defined in the header for performance — they are called frequently throughout the VM.
- `ms_values_equal` uses pointer equality for OBJ values because strings are interned. This is correct for the final design but relies on the VM's string interning table.
