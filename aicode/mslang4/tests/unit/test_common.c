#include "common.h"
#include <stdio.h>
#include <stdlib.h>

static void test_common_includes(void) {
    printf("common.h included successfully\n");
}

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

int main(void) {
    test_common_includes();
    test_stack_max();
    test_frames_max();
    test_gc_heap_grow_factor();
    test_max_locals();
    test_max_upvalues();
    test_table_max_load();
    test_result_values();
    test_result_is_enum();
    test_debug_macros_off();
    printf("All common definition tests passed.\n");
    return 0;
}
