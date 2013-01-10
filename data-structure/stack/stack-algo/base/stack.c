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
#include "stack.h"


struct stack_s {
  int limit;
  int top;
  element_t* elements;
};

#define STACK_LIMIT_DEF   (8)
#define ASSERT_STACK(s) {\
  if (NULL == (s)) {\
    fprintf(stderr, "stack invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_STACK_ELEMENT(s, over) {\
  if ((over)) {\
    if ((s)->top == (s)->limit) {\
      fprintf(stderr, "stack overflow ...");\
      exit(0);\
    }\
  }\
  else {\
    if (0 == (s)->top) {\
      fprintf(stderr, "stack underflow ...");\
      exit(0);\
    }\
  }\
}



stack_t 
stack_create(int limit) 
{
  stack_t s = (stack_t)malloc(sizeof(*s));
  ASSERT_STACK(s);

  s->limit = (limit < STACK_LIMIT_DEF ? STACK_LIMIT_DEF : limit);
  s->top = 0;
  s->elements = (element_t*)malloc(s->limit * sizeof(element_t));
  if (NULL == s->elements) {
    fprintf(stderr, "create stack failed ...");
    exit(0);
  }

  return s;
}

void 
stack_delete(stack_t* s)
{
  if (NULL != *s) {
    if (NULL != (*s)->elements)
      free((*s)->elements);
    free(*s);
    *s = NULL;
  }
}

int 
stack_empty(stack_t s)
{
  ASSERT_STACK(s);

  return (0 == s->top);
}

int 
stack_size(stack_t s)
{
  ASSERT_STACK(s);

  return (s->top);
}

void 
stack_push(stack_t s, element_t e)
{
  ASSERT_STACK(s);
  ASSERT_STACK_ELEMENT(s, 1);

  s->elements[s->top++] = e;
}

element_t
stack_pop(stack_t s)
{
  ASSERT_STACK(s);
  ASSERT_STACK_ELEMENT(s, 0);

  return s->elements[--s->top];
}

element_t 
stack_peek(stack_t s) 
{
  ASSERT_STACK(s);
  ASSERT_STACK_ELEMENT(s, 0);

  return s->elements[s->top - 1];
}
