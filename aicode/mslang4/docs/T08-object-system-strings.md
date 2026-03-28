# T08: Object System - Strings

**Phase**: 2 - Core Data Types
**Dependencies**: T07 (Value System)
**Estimated Complexity**: Medium

## Goal

Implement the base object system and string object type. This includes the `MsObject` header, string allocation (copy and take), hashing, concatenation, and object lifecycle management. String interning is deferred to the VM task.

## Files to Create

| File | Purpose |
|------|---------|
| `src/object.h` | Object types, all object structs (string only populated initially), macros |
| `src/object.c` | String operations, object free/print (string only initially) |

## TDD Implementation Cycles

### Cycle 1: MsObject Header and Type Enums

**RED** — Write failing test:

Create `tests/unit/test_object_string.c`. Write a test function `test_object_header` that:
- Creates a MsString via a helper (not yet available — will use direct allocation for now)
- Verifies the `MsObjectType` enum values exist
- Verifies the `MsObject` struct has `type`, `next`, and `isMarked` fields
- Verifies `MS_IS_STRING` and `MS_AS_STRING` macros compile and work

```c
// tests/unit/test_object_string.c (initial skeleton)
#include "object.h"
#include "value.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_object_header(void) {
    /* We'll create a string to test the header and macros */
    MsString* s = ms_string_copy("hello", 5);
    assert(s != NULL);
    assert(s->base.type == MS_OBJ_STRING);
    assert(s->base.next == NULL);
    assert(s->base.isMarked == false);
    assert(s->length == 5);
    assert(strncmp(s->chars, "hello", 5) == 0);
    assert(s->hash != 0);

    /* Test macros */
    MsValue val = ms_obj_val((MsObject*)s);
    assert(MS_IS_STRING(val));
    assert(MS_AS_STRING(val) == s);

    ms_object_free((MsObject*)s);
    printf("  test_object_header PASSED\n");
}

int main(void) {
    printf("Running object string tests...\n");
    test_object_header();
    printf("All object string tests passed.\n");
    return 0;
}
```

This will fail to compile because `object.h` does not exist.

**Verify RED**: `gcc -I src -o build/test_object_string tests/unit/test_object_string.c src/object.c src/value.c` → compilation error: `object.h: No such file or directory`

**GREEN** — Minimal implementation:

Create `src/object.h` with all object type definitions (forward-declare structs for later tasks):

```c
#ifndef MS_OBJECT_H
#define MS_OBJECT_H

#include "value.h"

typedef enum {
    MS_OBJ_STRING, MS_OBJ_FUNCTION, MS_OBJ_CLOSURE, MS_OBJ_UPVALUE,
    MS_OBJ_CLASS, MS_OBJ_INSTANCE, MS_OBJ_BOUND_METHOD, MS_OBJ_MODULE,
    MS_OBJ_LIST, MS_OBJ_NATIVE
} MsObjectType;

struct MsObject {
    MsObjectType type;
    MsObject* next;
    bool isMarked;
};

typedef struct { MsObject base; char* chars; int length; uint32_t hash; } MsString;

/* Forward declarations for later tasks */
typedef struct MsVM MsVM;
typedef MsValue (*MsNativeFn)(MsVM* vm, int argCount, MsValue* args);
typedef struct { MsObject base; MsNativeFn function; MsString* name; int arity; } MsNative;
typedef struct { MsObject base; int arity; int upvalueCount; char* _chunk_placeholder; MsString* name; } MsFunction;
typedef struct MsUpvalue { MsObject base; MsValue* location; MsValue closed; struct MsUpvalue* next; } MsUpvalue;
typedef struct { MsObject base; MsFunction* function; MsUpvalue** upvalues; int upvalueCount; } MsClosure;
typedef struct MsClass MsClass;
typedef struct { MsObject base; MsClass* klass; int _fields_placeholder; } MsInstance;
typedef struct { MsObject base; MsValue receiver; MsClosure* method; } MsBoundMethod;
typedef struct { MsObject base; MsString* name; char* path; int _exports_placeholder; bool isLoaded; } MsModule;
typedef struct { MsObject base; MsValue* elements; int count; int capacity; } MsList;

/* Macros */
#define MS_IS_STRING(v)  (ms_is_obj(v) && ms_as_obj(v)->type == MS_OBJ_STRING)
#define MS_AS_STRING(v)  ((MsString*)ms_as_obj(v))

MsString* ms_string_copy(const char* chars, int length);
MsString* ms_string_take(char* chars, int length);
uint32_t ms_string_hash(const char* key, int length);
MsString* ms_string_concat(MsString* a, MsString* b);
void ms_object_free(MsObject* obj);
void ms_object_print(MsValue value);

#endif
```

Create `src/object.c` with a minimal `ms_string_copy` implementation (enough to pass this test):

```c
#include "object.h"
#include "common.h"
#include <string.h>
#include <stdlib.h>

static MsString* ms_alloc_string(void) {
    MsString* str = MS_ALLOCATE(MsString, 1);
    str->base.type = MS_OBJ_STRING;
    str->base.next = NULL;
    str->base.isMarked = false;
    return str;
}

uint32_t ms_string_hash(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 2166136261u;
    }
    return hash;
}

MsString* ms_string_copy(const char* chars, int length) {
    uint32_t hash = ms_string_hash(chars, length);
    MsString* str = ms_alloc_string();
    str->chars = MS_ALLOCATE(char, length + 1);
    memcpy(str->chars, chars, length);
    str->chars[length] = '\0';
    str->length = length;
    str->hash = hash;
    return str;
}

void ms_object_free(MsObject* obj) {
    switch (obj->type) {
        case MS_OBJ_STRING: {
            MsString* str = (MsString*)obj;
            MS_FREE(char, str->chars, str->length + 1);
            MS_FREE(MsString, str, 1);
            break;
        }
        default: break;
    }
}
```

Design decision: Define ALL structs now (with placeholders for unimplemented fields) to avoid circular dependency issues later. Only implement string-related functions in this task.

Allocate `MsString` using `MS_ALLOCATE(MsString, 1)` and set `base.next = NULL`, `base.isMarked = false`. The GC-linked-list integration happens in T15/T17.

**Verify GREEN**: `gcc -I src -o build/test_object_string tests/unit/test_object_string.c src/object.c src/value.c && ./build/test_object_string` → test passes

**REFACTOR**: None needed.

---

### Cycle 2: FNV-1a Hash Function

**RED** — Write failing test:

Add `test_string_hash` to `test_object_string.c`:

```c
static void test_string_hash(void) {
    /* Empty string: FNV-1a offset basis */
    assert(ms_string_hash("", 0) == 2166136261u);

    /* Deterministic: same input always same hash */
    uint32_t h1 = ms_string_hash("hello", 5);
    uint32_t h2 = ms_string_hash("hello", 5);
    assert(h1 == h2);

    /* Different strings: different hashes (with high probability) */
    uint32_t h3 = ms_string_hash("world", 5);
    assert(h1 != h3);

    /* String created via copy has matching hash */
    MsString* s = ms_string_copy("hello", 5);
    assert(s->hash == h1);
    ms_object_free((MsObject*)s);

    printf("  test_string_hash PASSED\n");
}
```

**Verify RED**: Build and run — should already pass since `ms_string_hash` was implemented in Cycle 1. This is a verification cycle. If hash is not yet separately callable (e.g., declared static), this test would fail. Ensure `ms_string_hash` is declared in the header and exported.

**Verify GREEN**: `gcc -I src -o build/test_object_string tests/unit/test_object_string.c src/object.c src/value.c && ./build/test_object_string` → both tests pass.

**REFACTOR**: None needed.

---

### Cycle 3: String Take (Ownership Transfer)

**RED** — Write failing test:

Add `test_string_take` to `test_object_string.c`:

```c
static void test_string_take(void) {
    char* buffer = MS_ALLOCATE(char, 6);
    memcpy(buffer, "hello", 5);
    buffer[5] = '\0';

    MsString* s = ms_string_take(buffer, 5);
    assert(s != NULL);
    assert(s->base.type == MS_OBJ_STRING);
    assert(s->length == 5);
    assert(strcmp(s->chars, "hello") == 0);
    assert(s->hash == ms_string_hash("hello", 5));

    /* Verify it's the same pointer (took ownership, not copied) */
    assert(s->chars == buffer);

    ms_object_free((MsObject*)s);
    printf("  test_string_take PASSED\n");
}
```

**Verify RED**: Build fails — `ms_string_take` undefined.

**GREEN** — Minimal implementation:

Add to `src/object.c`:

```c
MsString* ms_string_take(char* chars, int length) {
    uint32_t hash = ms_string_hash(chars, length);
    MsString* str = ms_alloc_string();
    str->chars = chars;
    str->length = length;
    str->hash = hash;
    return str;
}
```

Takes ownership of existing char buffer, computes hash, wraps in MsString. Does NOT copy — the `chars` pointer is used directly.

**Verify GREEN**: Build and run — all three tests pass.

**REFACTOR**: None needed.

---

### Cycle 4: String Concatenation

**RED** — Write failing test:

Add `test_string_concat` to `test_object_string.c`:

```c
static void test_string_concat(void) {
    MsString* a = ms_string_copy("foo", 3);
    MsString* b = ms_string_copy("bar", 3);
    MsString* c = ms_string_concat(a, b);

    assert(c != NULL);
    assert(c->length == 6);
    assert(strcmp(c->chars, "foobar") == 0);
    assert(c->hash == ms_string_hash("foobar", 6));

    ms_object_free((MsObject*)c);
    ms_object_free((MsObject*)a);
    ms_object_free((MsObject*)b);
    printf("  test_string_concat PASSED\n");
}
```

**Verify RED**: Build fails — `ms_string_concat` undefined.

**GREEN** — Minimal implementation:

Add to `src/object.c`:

```c
MsString* ms_string_concat(MsString* a, MsString* b) {
    int length = a->length + b->length;
    char* chars = MS_ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';
    return ms_string_take(chars, length);
}
```

Allocates new buffer, copies a then b, takes ownership of new buffer.

**Verify GREEN**: Build and run — all four tests pass.

**REFACTOR**: None needed.

---

### Cycle 5: Object Print (String)

**RED** — Write failing test:

Add `test_object_print` to `test_object_string.c`:

```c
static void test_object_print(void) {
    MsString* s = ms_string_copy("hello world", 11);
    MsValue val = ms_obj_val((MsObject*)s);

    /* Visual check — print should output the string chars */
    printf("  Expect 'hello world': ");
    ms_object_print(val);
    printf("\n");

    ms_object_free((MsObject*)s);
    printf("  test_object_print PASSED (visual check)\n");
}
```

**Verify RED**: Build fails — `ms_object_print` undefined.

**GREEN** — Minimal implementation:

Add to `src/object.c`:

```c
#include <stdio.h>

void ms_object_print(MsValue value) {
    MsObject* obj = ms_as_obj(value);
    switch (obj->type) {
        case MS_OBJ_STRING: {
            MsString* str = (MsString*)obj;
            printf("%s", str->chars);
            break;
        }
        default:
            printf("<object %d>", obj->type);
            break;
    }
}
```

For strings, print `chars`. For others, print type name placeholder.

**Verify GREEN**: Build and run — all five tests pass.

**REFACTOR**: None needed.

---

### Cycle 6: Object Free and Hash Consistency

**RED** — Write failing test:

Add `test_object_free_and_consistency` to `test_object_string.c`:

```c
static void test_object_free_and_consistency(void) {
    /* Create and free multiple strings — no crash, no double-free */
    for (int i = 0; i < 10; i++) {
        char buf[16];
        int len = snprintf(buf, sizeof(buf), "str_%d", i);
        MsString* s = ms_string_copy(buf, len);
        assert(s->hash == ms_string_hash(buf, len));
        ms_object_free((MsObject*)s);
    }

    /* Hash consistency: different strings with same prefix */
    MsString* s1 = ms_string_copy("abc", 3);
    MsString* s2 = ms_string_copy("abcd", 4);
    assert(s1->hash != s2->hash);
    ms_object_free((MsObject*)s1);
    ms_object_free((MsObject*)s2);

    printf("  test_object_free_and_consistency PASSED\n");
}
```

**Verify RED**: Should already compile and pass since all functions are implemented. This is a verification/stress cycle.

**Verify GREEN**: `gcc -I src -o build/test_object_string tests/unit/test_object_string.c src/object.c src/value.c && ./build/test_object_string` → all tests pass.

**REFACTOR**: Run with address sanitizer to verify no leaks:
`gcc -fsanitize=address -I src -o build/test_object_string tests/unit/test_object_string.c src/object.c src/value.c && ./build/test_object_string`

## Acceptance Criteria

- [ ] `ms_string_copy("hello", 5)` creates a valid MsString with correct hash
- [ ] `ms_string_take(buffer, len)` takes ownership without copying
- [ ] `ms_string_hash("", 0)` returns the FNV-1a offset basis
- [ ] `ms_string_concat(a, b)` produces a string with contents "ab"
- [ ] `ms_object_free()` frees string without crash or leak
- [ ] `ms_object_print(ms_obj_val(str))` prints the string chars
- [ ] `MS_IS_STRING` macro works correctly
- [ ] Hash is deterministic: same input always produces same hash

## Notes

- `ms_alloc_object()` (from memory.h) requires a `MsVM*` for GC tracking. Since the VM doesn't exist yet, use `MS_ALLOCATE` directly for now and refactor in T15.
- All object type structs are defined now (with `_placeholder` fields for unimplemented ones) to avoid circular dependency issues in later tasks. Only string operations are implemented here.
- The `ms_object_free` switch handles `MS_OBJ_STRING` and has a `default: break` for all other types (handled in later tasks).
