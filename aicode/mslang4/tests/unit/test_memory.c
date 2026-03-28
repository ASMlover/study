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

int main(void) {
    test_alloc_free();
    test_realloc_grows();
    test_allocate_free_macro();
    test_grow_capacity();
    test_grow_array();
    printf("All memory tests passed.\n");
    return 0;
}
