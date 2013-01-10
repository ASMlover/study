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
#include <string.h>
#include "stack.h"

struct stack_t {
  int storage;
  int size;
  element_t* elements;
};

#define STORAGE_DEF   (32)
#define NEW(p)  ((p) = malloc(sizeof(*(p))))
#define NEW0(p) ((p) = calloc(1, sizeof(*(p))))
#define FREE(p) ((void)(free((void*)(p)), (p) = 0))

#define ASSERT_STACK(p) {\
  if (NULL == (p)) {\
    fprintf(stderr, "%s(%d) Error: stack object is null", __FILE__, __LINE__);\
    exit(0);\
  }\
}
#define ASSERT_STACK_VALID(p, s) {\
  if ((s) == (p)->size) {\
    fprintf(stderr, "%s(%d) Error: stack overflow", __FILE__, __LINE__);\
    exit(0);\
  }\
}


static void 
stack_regrow(stack_t s)
{
  int old_storage = s->storage;
  int new_storage = (0 == old_storage ? 1 : 2 * old_storage);
  element_t* new_elements = malloc(new_storage * sizeof(element_t));
  if (NULL == new_elements) {
    fprintf(stderr, "Error: create new stack elements failed");
    exit(0);
  }

  memcpy(new_elements, s->elements, sizeof(element_t) * old_storage);
  FREE(s->elements);
  s->elements = new_elements;
  s->storage  = new_storage;
}



stack_t 
stack_create(int storage_def)
{
  stack_t s;
  NEW(s);
  ASSERT_STACK(s);

  s->storage = (storage_def < STORAGE_DEF ? 
      STORAGE_DEF : storage_def);
  s->size    = 0;
  s->elements = malloc(storage_def * sizeof(element_t));
  if (NULL == s->elements) {
    FREE(s);
    fprintf(stderr, "Error: create stack elements failed");
    exit(0);
  }

  return s;
}

void 
stack_delete(stack_t* s)
{
  ASSERT_STACK(*s);

  if (NULL != (*s)->elements)
    FREE((*s)->elements);
  FREE(*s);
}

int 
stack_empty(stack_t s)
{
  ASSERT_STACK(s);

  return (0 == s->size);
}

int 
stack_storage(stack_t s)
{
  ASSERT_STACK(s);

  return s->storage;
}

int 
stack_storage_def(void)
{
  return STORAGE_DEF;
}

void
stack_push(stack_t s, element_t e)
{
  ASSERT_STACK(s);

  if (s->storage == s->size) 
    stack_regrow(s);
  s->elements[s->size++] = e;
}

element_t 
stack_pop(stack_t s)
{
  ASSERT_STACK(s);
  ASSERT_STACK_VALID(s, 0);

  return s->elements[--s->size];
}

element_t
stack_peek(stack_t s)
{
  int top;

  ASSERT_STACK(s);
  ASSERT_STACK_VALID(s, 0);

  top = s->size - 1;
  return s->elements[top];
}
