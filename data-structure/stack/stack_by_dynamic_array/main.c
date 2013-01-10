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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

static void 
stack_test_create(int storage) 
{
  stack_t s = stack_create(storage);
  int storage_def = stack_storage_def();
 
  if (storage < storage_def)
    assert(storage_def == stack_storage(s));
  else
    assert(storage == stack_storage(s));

  stack_delete(&s);

  fprintf(stdout, "Testing storage with %d success ...\n", storage);
}

static void 
stack_test_empty(int is_push_to_stack)
{
  stack_t s = stack_create(0);

  if (!is_push_to_stack) 
    assert(stack_empty(s));
  else {
    double* d = (double*)malloc(sizeof(double));
    *d = 34.56;
    stack_push(s, d);
    assert(!stack_empty(s));

    d = stack_pop(s);
    free(d);
  }

  stack_delete(&s);

  fprintf(stdout, "Testing stack_empty success ...\n");
}

static void 
stack_test_push(element_t e, 
  int (*cmp)(element_t, element_t, int), int elem_size) 
{
  stack_t s = stack_create(0);
  element_t pop_e;

  stack_push(s, e);
  pop_e = stack_pop(s);
  assert(cmp(e, pop_e, elem_size));

  stack_delete(&s);
  fprintf(stdout, "Testing stack_push with element success ...\n");
}

static void 
stack_test_peek(element_t e, 
  int (*cmp)(element_t, element_t, int), int elem_size)
{
  stack_t s = stack_create(0);
  element_t peek_e;

  stack_push(s, e);
  peek_e = stack_peek(s);
  assert(cmp(e, peek_e, elem_size));

  stack_delete(&s);
  fprintf(stdout, "Testing stack_peek with element success ...\n");
}




static int 
element_cmp(element_t a, element_t b, int elem_size)
{
  return (0 == memcmp(a, b, elem_size));
}

int 
main(int argc, char* argv[])
{
  int* val_n;
  double* val_d;

  stack_test_create(5);
  stack_test_create(10);
  stack_test_create(33);
  stack_test_create(100);

  stack_test_empty(1);
  stack_test_empty(0);

  val_n = (int*)malloc(sizeof(int));
  *val_n = 45;
  stack_test_push(val_n, element_cmp, sizeof(int));
  stack_test_peek(val_n, element_cmp, sizeof(int));
  free(val_n);
  val_d = (double*)malloc(sizeof(double));
  *val_d = 34.56;
  stack_test_push(val_d, element_cmp, sizeof(double));
  stack_test_peek(val_d, element_cmp, sizeof(double));
  free(val_d);

  return 0;
}
