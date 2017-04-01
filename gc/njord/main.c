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
#include "njmem.h"

#define ALLOC_COUNT (100000)

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  int* p[ALLOC_COUNT];
  clock_t beg = clock();
  clock_t end = beg;
  fprintf(stdout, "[system allocator] begin clock: %ld\n", beg);
  for (int i = 0; i < ALLOC_COUNT; ++i)
    p[i] = (int*)malloc(sizeof(int));
  for (int i = 0; i < ALLOC_COUNT; ++i)
    free(p[i]);
  end = clock();
  fprintf(stdout, "[system allocator] end clock: %ld,  use clock: %ld\n", end, end - beg);

  {
    beg = clock();
    fprintf(stdout, "[mempool allocator] begin clock: %ld\n", beg);
    for (int i = 0; i < ALLOC_COUNT; ++i)
      p[i] = (int*)njmem_malloc(sizeof(int));
    for (int i = 0; i < ALLOC_COUNT; ++i)
      njmem_free(p[i], sizeof(int));
    end = clock();
    fprintf(stdout, "[mempool allocator] end clock: %ld,  use clock: %ld\n", end, end - beg);
  }

  njmem_collect();
  return 0;
}
