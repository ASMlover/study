/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
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
#include "array.h"


#define ARRAY_TEST_PUSH(a, f) {\
  fprintf(stdout, "\narray test ==> " #f ":\n");\
  array_test_push((a), (f));\
}
#define ARRAY_TEST_POP(a, f) {\
  fprintf(stdout, "\narray test ==> " #f ":\n");\
  array_test_pop((a), (f));\
}

static void 
array_test_push(array_t a, void (*push)(array_t, element_t)) 
{
  int i = 0;
  double* d;

  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = (i + i) * 0.123;
    push(a, d);

    fprintf(stdout, "array push back {index=>%d, value=>%lf}\n", i, *d);
  }
  fprintf(stdout, "array size is : %d\n", array_size(a));

  for (i = 0; i < 10; ++i) {
    d = (double*)array_get(a, i);
    fprintf(stdout, "array get {index=>%d, value=>%lf}\n", i, *d);
    free(d);
  }

  array_clear(a);
  fprintf(stdout, "array size is : %d\n", array_size(a));
}

static void 
array_test_pop(array_t a, element_t (*pop)(array_t)) 
{
  int i;
  double* d;

  for (i = 0; i < 10; ++i) {
    d = (double*)malloc(sizeof(*d));
    *d = (i * i) * 0.34219;
    fprintf(stdout, "push array element {index=>%d, value=>%lf}\n", i, *d);
    array_push_back(a, d);
  }
  fprintf(stdout, "array size is : %d\n", array_size(a));

  while (!array_empty(a)) {
    d = (double*)pop(a);
    fprintf(stdout, "the poped element {value=>%lf}\n", *d);
    free(d);
  }
  fprintf(stdout, "array size is : %d\n", array_size(a));
}



int 
main(int argc, char* argv[])
{
  array_t a = array_create(0);

  ARRAY_TEST_PUSH(a, array_push_front);
  ARRAY_TEST_PUSH(a, array_push_back);
  ARRAY_TEST_POP(a, array_pop_front);
  ARRAY_TEST_POP(a, array_pop_back);

  array_delete(&a);
  return 0;
}
