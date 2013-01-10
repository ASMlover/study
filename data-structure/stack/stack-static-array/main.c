/*
 * Copyright (c) 2012 ASMlover. All rights reserved.
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
#include <time.h>
#include <string.h>
#include "stack.h"


static void 
stack_test_create(int limit)
{
  stack_t s = stack_create(limit);

  if (limit < 8)
    assert(8 == stack_limit(s));
  else
    assert(limit == stack_limit(s));

  assert(stack_empty(s));

  stack_delete(&s);
  fprintf(stdout, "Testing stack with limit [%d] success ...\n", limit);
}

static void 
stack_test_push(element_t e, 
  int (*cmp)(element_t, element_t, int), int elem_size)
{
  stack_t s = stack_create(0);
  element_t pop_elem;

  stack_push(s, e);
  pop_elem = stack_pop(s);

  assert(cmp(e, pop_elem, elem_size));

  stack_delete(&s);
  fprintf(stdout, "Testing stack push with element success ...\n");
}

static void 
stack_test_peek(element_t e, 
  int (*cmp)(element_t, element_t, int), int elem_size)
{
  stack_t s = stack_create(0);
  element_t peek_elem;

  stack_push(s, e);
  peek_elem = stack_pop(s);

  assert(cmp(e, peek_elem, elem_size));

  stack_delete(&s);
  fprintf(stdout, "Testing stack peek with element success ...\n");
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

  stack_test_create(3);
  stack_test_create(10);
  stack_test_create(11);
  stack_test_create(0);

  val_n = (int*)malloc(sizeof(int));
  *val_n = rand() % 23;
  stack_test_push(val_n, element_cmp, sizeof(int));
  stack_test_peek(val_n, element_cmp, sizeof(int));
  free(val_n);

  val_d = (double*)malloc(sizeof(double));
  *val_d = rand() % 234 * 0.123;
  stack_test_push(val_d, element_cmp, sizeof(double));
  stack_test_peek(val_d, element_cmp, sizeof(double));
  free(val_d);

  return 0;
}
