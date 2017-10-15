/*
 * Copyright (c) 2017 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pymem.h"

extern void* PyObject_Malloc(size_t nbytes);
extern void PyObject_Free(void* p);

static const int ALLOC_COUNT = 1000000;
static const int TEST_COUNT = 100;

static void bench_test1(int alloc_size_array[]) {
  int count = 0;

  fprintf(stdout, "#################### [bench_test1] ####################\n");
  while (count++ < TEST_COUNT) {
    void* p;
    clock_t beg = clock();
    clock_t end;
    for (int i = 0; i < ALLOC_COUNT; ++i) {
      p = malloc(alloc_size_array[i]);
      free(p);
    }
    end = clock();
    fprintf(stdout,
        "[system allocator][%d] %ld ~ %ld, used: %ld\n",
        count, beg, end, end - beg);

    beg = clock();
    for (int i = 0; i < ALLOC_COUNT; ++i) {
#if defined(PYMEM_PYTHON)
      p = PyObject_Malloc(alloc_size_array[i]);
      PyObject_Free(p);
#else
      p = pymem_alloc(alloc_size_array[i]);
      pymem_dealloc(p);
#endif
    }
    end = clock();
    fprintf(stdout,
        "[pymem allocator][%d] %ld ~ %ld, used: %ld\n",
        count, beg, end, end - beg);
  }
}

static void bench_test2(int alloc_size_array[]) {
  int count = 0;

  fprintf(stdout, "#################### [bench_test2] ####################\n");
  while (count++ < TEST_COUNT) {
    void** alloc_memory_list = (void**)malloc(sizeof(void*) * ALLOC_COUNT);
    clock_t beg = clock();
    clock_t end;
    for (int i = 0; i < ALLOC_COUNT; ++i) {
      alloc_memory_list[i] = malloc(alloc_size_array[i]);
    }
    for (int i = 0; i < ALLOC_COUNT; ++i)
      free(alloc_memory_list[i]);
    end = clock();
    fprintf(stdout,
        "[system allocator][%d] %ld ~ %ld, used: %ld\n",
        count, beg, end, end - beg);

    beg = clock();
    for (int i = 0; i < ALLOC_COUNT; ++i) {
#if defined(PYMEM_PYTHON)
      alloc_memory_list[i] = PyObject_Malloc(alloc_size_array[i]);
#else
      alloc_memory_list[i] = pymem_alloc(alloc_size_array[i]);
#endif
    }
    for (int i = 0; i < ALLOC_COUNT; ++i) {
#if defined(PYMEM_PYTHON)
      PyObject_Free(alloc_memory_list[i]);
#else
      pymem_dealloc(alloc_memory_list[i]);
#endif
    }
    end = clock();
    fprintf(stdout,
        "[pymem allocator][%d] %ld ~ %ld, used: %ld\n",
        count, beg, end, end - beg);

    free(alloc_memory_list);
  }
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

#if defined(PYMEM_PYTHON)
  fprintf(stdout, "memory pool testing: use python allocating ...\n");
#else
  fprintf(stdout, "memory pool testing: use self allocating ...\n");
#endif

  int* alloc_array = (int*)malloc(ALLOC_COUNT * sizeof(int));

  srand((unsigned int)time(NULL));
  for (int i = 0; i < ALLOC_COUNT; ++i) {
    alloc_array[i] = rand() % 512;
    if (alloc_array[i] == 0)
      alloc_array[i] = 1;
  }

  bench_test1(alloc_array);
  bench_test2(alloc_array);

  free(alloc_array);

  return 0;
}
