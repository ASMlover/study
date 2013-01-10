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
  element_t* elements;
  int*       tops;
  int        num;
  int        plimit;
};

#define MIN_PER_STACK   (8)


stack_t 
stack_create(int num, int per_len)
{
  int i;
  stack_t s = (stack_t)malloc(sizeof(*s));

  if (NULL == s) {
    fprintf(stderr, "create stack failed ...");
    exit(0);
  }
  s->num = (0 == num ? 1 : num);
  s->tops = (int*)malloc(num * sizeof(int));
  if (NULL == s->tops) {
    fprintf(stderr, "create stack failed ...");
    exit(0);
  }
  s->plimit = (MIN_PER_STACK > per_len ? MIN_PER_STACK : per_len);
  for (i = 0; i < s->num; ++i) 
    s->tops[i] = 0 + i * per_len;

  s->elements = (element_t*)malloc(s->plimit * num * sizeof(element_t));
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
    if (NULL != (*s)->tops)
      free((*s)->tops);
    if (NULL != (*s)->elements)
      free((*s)->elements);

    free(*s);
    *s = NULL;
  }
}

int 
stack_empty(stack_t s, int index)
{
  if (0 == s || index < 0 || index >= s->num) {
    fprintf(stderr, "stack invalid ...");
    exit(0);
  }

  return (s->tops[index] == index * s->plimit);
}

int 
stack_size(stack_t s, int index)
{
  if (0 == s || index < 0 || index >= s->num) {
    fprintf(stderr, "stack invalid ...");
    exit(0);
  }

  return (s->tops[index] - index * s->plimit);
}

void 
stack_push(stack_t s, int index, element_t e)
{
  if (0 == s || index < 0 || index >= s->num) {
    fprintf(stderr, "stack invalid ...");
    exit(0);
  }
  if (s->plimit == s->tops[index] - index * s->plimit) {
    fprintf(stderr, "stack overflow ...");
    exit(0);
  }

  s->elements[s->tops[index]++] = e;
}

element_t 
stack_pop(stack_t s, int index) 
{
  if (0 == s || index < 0 || index >= s->num) {
    fprintf(stderr, "stack invalid ...");
    exit(0);
  }
  if (0 == s->tops[index] - index * s->plimit) {
    fprintf(stderr, "stack underflow ...");
    exit(0);
  }

  return s->elements[--s->tops[index]];
}

element_t 
stack_peek(stack_t s, int index)
{
  if (0 == s || index < 0 || index >= s->num) {
    fprintf(stderr, "stack invalid ...");
    exit(0);
  }
  if (0 == s->tops[index] - index * s->plimit) {
    fprintf(stderr, "stack underflow ...");
    exit(0);
  }

  return s->elements[s->tops[index] - 1];
}
