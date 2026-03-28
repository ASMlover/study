# T02: Common Definitions

**Phase**: 1 - Foundation
**Dependencies**: T01 (Project Skeleton)
**Estimated Complexity**: Low
**Status**: ✅ COMPLETED

## Goal

Define all shared constants, result types, and debug macros used across the entire project. This is the foundational header included by every other source file.

## Files to Create/Modify

| File | Action |
|------|--------|
| `src/common.h` | Fill in with complete definitions |
| `tests/unit/test_common.c` | Unit tests for common definitions |
| `tests/CMakeLists.txt` | Add test_common target |

## TDD Implementation Cycles

### Cycle 1: Include Guards and Standard Includes — Compiles Standalone

**RED** — Write failing test:
- Create `tests/unit/test_common.c` that includes `common.h` and calls `printf`. This will fail to link if `common.h` has syntax errors, or fail to compile if standard headers are missing.

```c
#include "common.h"
#include <stdio.h>

static void test_common_includes(void) {
    printf("common.h included successfully\n");
}

int main(void) {
    test_common_includes();
    return 0;
}
```

- Add to `tests/CMakeLists.txt`:

```cmake
add_executable(test_common tests/unit/test_common.c)
target_include_directories(test_common PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME test_common COMMAND test_common)
```

- `common.h` is currently an empty placeholder from T01 — the test compiles but has no real assertions yet.

**Verify RED**: 
```
cmake --build build
ctest --test-dir build -R test_common
```
Expected: passes trivially (no assertions to fail). This establishes the test infrastructure is working.

**GREEN** — Minimal implementation:
- Fill `src/common.h` with include guards and standard includes only:

```c
#ifndef MS_COMMON_H
#define MS_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#endif
```

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_common
```
Expected: compiles and test passes

**REFACTOR**: No changes needed.

### Cycle 2: Compile-Time Constants

**RED** — Write failing test:
- Add test functions to `tests/unit/test_common.c` that assert on constant values:

```c
static void test_stack_max(void) {
    if (MS_STACK_MAX != 256) {
        fprintf(stderr, "FAIL: MS_STACK_MAX expected 256, got %d\n", MS_STACK_MAX);
        exit(1);
    }
}

static void test_frames_max(void) {
    if (MS_FRAMES_MAX != 64) {
        fprintf(stderr, "FAIL: MS_FRAMES_MAX expected 64, got %d\n", MS_FRAMES_MAX);
        exit(1);
    }
}

static void test_gc_heap_grow_factor(void) {
    if (MS_GC_HEAP_GROW_FACTOR != 2) {
        fprintf(stderr, "FAIL: MS_GC_HEAP_GROW_FACTOR expected 2, got %d\n", MS_GC_HEAP_GROW_FACTOR);
        exit(1);
    }
}

static void test_max_locals(void) {
    if (MS_MAX_LOCALS != 256) {
        fprintf(stderr, "FAIL: MS_MAX_LOCALS expected 256, got %d\n", MS_MAX_LOCALS);
        exit(1);
    }
}

static void test_max_upvalues(void) {
    if (MS_MAX_UPVALUES != 256) {
        fprintf(stderr, "FAIL: MS_MAX_UPVALUES expected 256, got %d\n", MS_MAX_UPVALUES);
        exit(1);
    }
}

static void test_table_max_load(void) {
    if (MS_TABLE_MAX_LOAD != 0.75) {
        fprintf(stderr, "FAIL: MS_TABLE_MAX_LOAD expected 0.75, got %f\n", MS_TABLE_MAX_LOAD);
        exit(1);
    }
}
```

- Call all from `main()`.
- `MS_STACK_MAX` etc. are not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `MS_STACK_MAX` undeclared (and others)

**GREEN** — Minimal implementation:
- Add constants to `src/common.h` (after the standard includes, before `#endif`):

```c
#define MS_STACK_MAX    256
#define MS_FRAMES_MAX   64
#define MS_GC_HEAP_GROW_FACTOR 2
#define MS_MAX_LOCALS   256
#define MS_MAX_UPVALUES 256
#define MS_TABLE_MAX_LOAD 0.75
```

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_common
```
Expected: compiles and all assertions pass

**REFACTOR**: No changes needed.

### Cycle 3: MsResult Enum

**RED** — Write failing test:
- Add test functions that verify `MsResult` enum exists and has correct values:

```c
static void test_result_values(void) {
    if (MS_OK != 0) {
        fprintf(stderr, "FAIL: MS_OK expected 0, got %d\n", MS_OK);
        exit(1);
    }
    if (MS_COMPILE_ERROR != 1) {
        fprintf(stderr, "FAIL: MS_COMPILE_ERROR expected 1, got %d\n", MS_COMPILE_ERROR);
        exit(1);
    }
    if (MS_RUNTIME_ERROR != 2) {
        fprintf(stderr, "FAIL: MS_RUNTIME_ERROR expected 2, got %d\n", MS_RUNTIME_ERROR);
        exit(1);
    }
}

static void test_result_is_enum(void) {
    MsResult result = MS_OK;
    (void)result;
}
```

- `MsResult` is not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `MsResult` and `MS_OK` undeclared

**GREEN** — Minimal implementation:
- Add `MsResult` enum to `src/common.h`:

```c
typedef enum { MS_OK, MS_COMPILE_ERROR, MS_RUNTIME_ERROR } MsResult;
```

Design decisions:
- `MsResult` is the universal return type for operations that can fail
- Values start at 0 (C default) and are sequential

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_common
```
Expected: compiles and all assertions pass

**REFACTOR**: No changes needed.

### Cycle 4: Debug Macros

**RED** — Write failing test:
- Add test functions that verify debug macros behave correctly both when the flag is defined and when it is not:

```c
static int debug_gc_counter;

static void test_debug_macros_off(void) {
    debug_gc_counter = 0;
    MS_DEBUG_LOG_GC_EXECUTE(debug_gc_counter++);
    if (debug_gc_counter != 0) {
        fprintf(stderr, "FAIL: MS_DEBUG_LOG_GC_EXECUTE should be no-op when flag not defined\n");
        exit(1);
    }

    MS_DEBUG_STRESS_GC_EXECUTE(debug_gc_counter++);
    if (debug_gc_counter != 0) {
        fprintf(stderr, "FAIL: MS_DEBUG_STRESS_GC_EXECUTE should be no-op when flag not defined\n");
        exit(1);
    }

    MS_DEBUG_TRACE_EXECUTE(debug_gc_counter++);
    if (debug_gc_counter != 0) {
        fprintf(stderr, "FAIL: MS_DEBUG_TRACE_EXECUTE should be no-op when flag not defined\n");
        exit(1);
    }
}
```

- `MS_DEBUG_LOG_GC_EXECUTE` etc. are not defined yet — compile error.

**Verify RED**: 
```
cmake --build build
```
Expected: compile error — `MS_DEBUG_LOG_GC_EXECUTE` undeclared

**GREEN** — Minimal implementation:
- Add debug macros to `src/common.h`:

```c
#ifdef MS_DEBUG_LOG_GC
  #define MS_DEBUG_LOG_GC_EXECUTE(code) code
#else
  #define MS_DEBUG_LOG_GC_EXECUTE(code)
#endif

#ifdef MS_DEBUG_STRESS_GC
  #define MS_DEBUG_STRESS_GC_EXECUTE(code) code
#else
  #define MS_DEBUG_STRESS_GC_EXECUTE(code)
#endif

#ifdef MS_DEBUG_TRACE_EXECUTION
  #define MS_DEBUG_TRACE_EXECUTE(code) code
#else
  #define MS_DEBUG_TRACE_EXECUTE(code)
#endif
```

Design decisions:
- Debug macros use the execute-pattern to avoid evaluating code in release builds
- The `code` argument is only expanded when the corresponding flag is defined

**Verify GREEN**: 
```
cmake --build build
ctest --test-dir build -R test_common
```
Expected: compiles and test passes (macros expand to nothing when flags not defined)

**REFACTOR**: Verify the test also compiles when flags ARE defined. A separate build with `-DMS_DEBUG_LOG_GC` can confirm the macro expands correctly, but this is optional for now. Add a comment noting that when `MS_DEBUG_LOG_GC` is defined, `MS_DEBUG_LOG_GC_EXECUTE(debug_gc_counter++)` should increment the counter.

## Acceptance Criteria

- [x] `common.h` compiles when included from a `.c` file
- [x] All constants are defined with correct values
- [x] `MsResult` enum has exactly 3 values
- [x] Debug macros expand to nothing when their flag is not defined
- [x] Debug macros expand to their argument when the flag IS defined
- [x] No warnings with strict compilation flags

## Notes

- `common.h` is a header-only module — no `.c` file needed.
- The constants (`MS_STACK_MAX`, `MS_FRAMES_MAX`, etc.) are compile-time constants for fixed-stack allocation.
- Debug macros will be used by the GC (T17) and VM tracing (T14).
