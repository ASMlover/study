# T03: Memory Subsystem

**Phase**: 1 - Foundation
**Dependencies**: T02 (Common Definitions)
**Estimated Complexity**: Low

## Goal

Implement the centralized memory allocation wrapper and utility macros. All heap allocation in the project MUST go through `ms_reallocate()` to enable GC tracking and leak detection.

## Files to Create

| File | Purpose |
|------|---------|
| `src/memory.h` | Memory API and macros |
| `src/memory.c` | Memory implementation |
| `tests/unit/test_memory.c` | Unit tests for memory subsystem |

## TDD Implementation Cycles

### Cycle 1: ms_reallocate — Allocate and Free

**RED** — Write failing test:
- Create `tests/unit/test_memory.c` with a test for basic allocation and freeing:

```c
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_alloc_free(void) {
    void* ptr = ms_reallocate(NULL, 0, 100);
    if (ptr == NULL) {
        fprintf(stderr, "FAIL: ms_reallocate(NULL, 0, 100) returned NULL\n");
        exit(1);
    }
    memset(ptr, 0xAB, 100);

    void* ptr2 = ms_reallocate(ptr, 100, 0);
    if (ptr2 != NULL) {
        fprintf(stderr, "FAIL: ms_reallocate(ptr, 100, 0) should return NULL\n");
        exit(1);
    }
}

int main(void) {
    test_alloc_free();
    printf("test_alloc_free passed\n");
    return 0;
}
```

- Add to `tests/CMakeLists.txt`:
```cmake
add_executable(test_memory tests/unit/test_memory.c)
target_include_directories(test_memory PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_memory PRIVATE maple)
add_test(NAME test_memory COMMAND test_memory)
```

- `memory.h` and `memory.c` do not exist yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `memory.h` not found

**GREEN** — Minimal implementation:
- Create `src/memory.h`:

```c
#ifndef MS_MEMORY_H
#define MS_MEMORY_H

#include "common.h"

void* ms_reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif
```

- Create `src/memory.c`:

```c
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

void* ms_reallocate(void* pointer, size_t oldSize, size_t newSize) {
    (void)oldSize;
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) {
        fprintf(stderr, "Maple: out of memory\n");
        exit(1);
    }
    return result;
}
```

Key behaviors:
- `ms_reallocate(NULL, 0, size)` — equivalent to `malloc(size)`
- `ms_reallocate(ptr, oldSize, newSize)` — equivalent to `realloc(ptr, newSize)`
- `ms_reallocate(ptr, oldSize, 0)` — equivalent to `free(ptr)`, returns NULL
- Exits on allocation failure (simple but safe for a scripting language)
- The `oldSize`/`newSize` parameters are for future GC tracking (bytes allocated)

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_memory
```
Expected: compiles and test passes

**REFACTOR**: The `(void)oldSize;` suppresses the unused-parameter warning. `oldSize` will be used by GC in T17.

### Cycle 2: ms_reallocate — Grow in Place

**RED** — Write failing test:
- Add a test that verifies reallocation preserves existing data:

```c
static void test_realloc_grows(void) {
    int* arr = (int*)ms_reallocate(NULL, 0, 5 * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "FAIL: initial alloc returned NULL\n");
        exit(1);
    }

    for (int i = 0; i < 5; i++) {
        arr[i] = i * 10;
    }

    arr = (int*)ms_reallocate(arr, 5 * sizeof(int), 10 * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "FAIL: realloc returned NULL\n");
        exit(1);
    }

    for (int i = 0; i < 5; i++) {
        if (arr[i] != i * 10) {
            fprintf(stderr, "FAIL: data not preserved after realloc at index %d\n", i);
            exit(1);
        }
    }

    ms_reallocate(arr, 10 * sizeof(int), 0);
}
```

- Call from `main()`. This test should already pass with the existing `ms_reallocate` implementation — it validates that `realloc` behavior is correct.

**Verify RED**: 
```
cmake --build build
ctest --test-dir build -R test_memory
```
Expected: passes (this is a validation test — the implementation from Cycle 1 already supports this)

**GREEN** — No changes needed. The implementation from Cycle 1 already handles this case.

**Verify GREEN**: Same as above — test passes.

**REFACTOR**: No changes needed.

### Cycle 3: MS_ALLOCATE and MS_FREE Macros

**RED** — Write failing test:
- Add a test for the typed allocation/free macros:

```c
static void test_allocate_free_macro(void) {
    int* arr = MS_ALLOCATE(int, 10);
    if (arr == NULL) {
        fprintf(stderr, "FAIL: MS_ALLOCATE returned NULL\n");
        exit(1);
    }

    for (int i = 0; i < 10; i++) {
        arr[i] = i;
    }
    for (int i = 0; i < 10; i++) {
        if (arr[i] != i) {
            fprintf(stderr, "FAIL: MS_ALLOCATE data mismatch at %d\n", i);
            exit(1);
        }
    }

    MS_FREE(int, arr, 10);
}
```

- `MS_ALLOCATE` and `MS_FREE` are not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `MS_ALLOCATE` undeclared

**GREEN** — Minimal implementation:
- Add macros to `src/memory.h`:

```c
#define MS_ALLOCATE(type, count) \
    (type*)ms_reallocate(NULL, 0, sizeof(type) * (count))
#define MS_FREE(type, pointer, count) \
    ms_reallocate(pointer, sizeof(type) * (count), 0)
```

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_memory
```
Expected: compiles and test passes

**REFACTOR**: No changes needed.

### Cycle 4: MS_GROW_CAPACITY Macro

**RED** — Write failing test:
- Add a test for the capacity-growth macro at boundary values:

```c
static void test_grow_capacity(void) {
    if (MS_GROW_CAPACITY(0) != 8) {
        fprintf(stderr, "FAIL: MS_GROW_CAPACITY(0) expected 8, got %d\n", MS_GROW_CAPACITY(0));
        exit(1);
    }
    if (MS_GROW_CAPACITY(1) != 8) {
        fprintf(stderr, "FAIL: MS_GROW_CAPACITY(1) expected 8\n");
        exit(1);
    }
    if (MS_GROW_CAPACITY(7) != 8) {
        fprintf(stderr, "FAIL: MS_GROW_CAPACITY(7) expected 8\n");
        exit(1);
    }
    if (MS_GROW_CAPACITY(8) != 16) {
        fprintf(stderr, "FAIL: MS_GROW_CAPACITY(8) expected 16, got %d\n", MS_GROW_CAPACITY(8));
        exit(1);
    }
    if (MS_GROW_CAPACITY(16) != 32) {
        fprintf(stderr, "FAIL: MS_GROW_CAPACITY(16) expected 32\n");
        exit(1);
    }
    if (MS_GROW_CAPACITY(100) != 200) {
        fprintf(stderr, "FAIL: MS_GROW_CAPACITY(100) expected 200\n");
        exit(1);
    }
}
```

- `MS_GROW_CAPACITY` is not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `MS_GROW_CAPACITY` undeclared

**GREEN** — Minimal implementation:
- Add macro to `src/memory.h`:

```c
#define MS_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)
```

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_memory
```
Expected: compiles and test passes

**REFACTOR**: No changes needed.

### Cycle 5: MS_GROW_ARRAY and MS_FREE_ARRAY Macros

**RED** — Write failing test:
- Add a test for the array grow/free macros that verifies contents are preserved after growth:

```c
static void test_grow_array(void) {
    int* arr = MS_ALLOCATE(int, 4);
    for (int i = 0; i < 4; i++) {
        arr[i] = i * 100;
    }

    arr = MS_GROW_ARRAY(int, arr, 4, 8);
    if (arr == NULL) {
        fprintf(stderr, "FAIL: MS_GROW_ARRAY returned NULL\n");
        exit(1);
    }

    for (int i = 0; i < 4; i++) {
        if (arr[i] != i * 100) {
            fprintf(stderr, "FAIL: MS_GROW_ARRAY data not preserved at %d\n", i);
            exit(1);
        }
    }

    MS_FREE_ARRAY(int, arr, 8);
}
```

- `MS_GROW_ARRAY` and `MS_FREE_ARRAY` are not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `MS_GROW_ARRAY` undeclared

**GREEN** — Minimal implementation:
- Add macros to `src/memory.h`:

```c
#define MS_GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)ms_reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))
#define MS_FREE_ARRAY(type, pointer, oldCount) \
    ms_reallocate(pointer, sizeof(type) * (oldCount), 0)
```

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_memory
```
Expected: compiles and test passes

**REFACTOR**: Review final `src/memory.h` — all 5 macros are now defined. The complete header matches the spec from DESIGN.md §2.14.

## Acceptance Criteria

- [ ] `ms_reallocate(NULL, 0, 100)` allocates 100 bytes and returns non-NULL
- [ ] `ms_reallocate(ptr, 100, 200)` reallocates to 200 bytes
- [ ] `ms_reallocate(ptr, 200, 0)` frees and returns NULL
- [ ] `MS_ALLOCATE(int, 10)` allocates array of 10 ints
- [ ] `MS_FREE(int, ptr, 10)` frees the array
- [ ] `MS_GROW_CAPACITY(0) == 8`, `MS_GROW_CAPACITY(8) == 16`, `MS_GROW_CAPACITY(16) == 32`
- [ ] All macros compile and work correctly
- [ ] No memory leaks (verifiable with valgrind/sanitizers)

## Notes

- GC functions (`ms_gc_collect`, `ms_gc_mark_*`, etc.) will be added in T17 (Garbage Collection). This task only covers the allocation wrapper.
- The `oldSize` parameter in `ms_reallocate` is currently unused — it will track bytes allocated for GC heap management in T17.
- All returned pointers from allocation macros must be freed using the corresponding `MS_FREE`/`MS_FREE_ARRAY` macros.
