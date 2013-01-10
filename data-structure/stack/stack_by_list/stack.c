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


typedef struct stack_node_t {
  element_t e;
  struct stack_node_t* next;
} *stack_node_t;

struct stack_t {
  int size;
  stack_node_t head;
};
#define NEW(p)  ((p) = malloc(sizeof(*(p))))
#define NEW0(p) ((p) = calloc(1, sizeof(*(p))))
#define FREE(p) ((void)(free((void*)(p)), (p) = 0))

#define ASSERT_STACK(p) {\
  if (NULL == (p)) {\
    fprintf(stderr, "%s(%d) Error: stack object is null", __FILE__, __LINE__);\
    exit(0);\
  }\
}
#define ASSERT_STACK_VALID(p) {\
  if (NULL == (p)->head) {\
    fprintf(stderr, "%s(%d) Error: stack overflow", __FILE__, __LINE__);\
    exit(0);\
  }\
}


static stack_node_t 
stack_node_create(element_t e)
{
  stack_node_t node;
  NEW(node);

  if (NULL == node) {
    fprintf(stderr, "Error: create stack node failed");
    exit(0);
  }
  node->e = e;

  return node;
}

static void 
stack_clear(stack_t s)
{
  stack_node_t iter = s->head;
  stack_node_t node;

  while (NULL != iter) {
    node = iter;
    iter = iter->next;
    FREE(node);
  }
  s->size = 0;
}



stack_t 
stack_create(void)
{
  stack_t s;
  NEW(s);
  ASSERT_STACK(s);

  s->size = 0;
  s->head = NULL;

  return s;
}

void 
stack_delete(stack_t* s)
{
  ASSERT_STACK(*s);
  
  stack_clear(*s);
  FREE(*s);
}

int 
stack_empty(stack_t s)
{
  ASSERT_STACK(s);
  return (NULL == s->head);
}

int 
stack_size(stack_t s)
{
  ASSERT_STACK(s);
  return s->size;
}

void 
stack_push(stack_t s, element_t e)
{
  stack_node_t node;
  ASSERT_STACK(s);

  node = stack_node_create(e);
  node->next = s->head;
  s->head = node;
  ++s->size;
}

element_t
stack_pop(stack_t s)
{
  stack_node_t pop_node;
  element_t    pop_elem;
  ASSERT_STACK(s);
  ASSERT_STACK_VALID(s);

  pop_node = s->head;
  s->head = s->head->next;
  pop_elem = pop_node->e;
  free(pop_node);
  --s->size;

  return pop_elem;
}

element_t
stack_peek(stack_t s)
{
  ASSERT_STACK(s);
  ASSERT_STACK_VALID(s);

  return s->head->e;
}
