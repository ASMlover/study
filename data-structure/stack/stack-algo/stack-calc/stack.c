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


#define ASSERT_STACK(s) {\
  if (NULL == (s)) {\
    fprintf(stderr, "stack invalid ...");\
    exit(0);\
  }\
}

#define ASSERT_STACK_ELEMENT(s) {\
  if (NULL == (s)->head) {\
    fprintf(stderr, "stack underflow ...");\
    exit(0);\
  }\
}


typedef struct stack_node_s {
  element_t e;
  struct stack_node_s* next;
} *stack_node_t;

struct stack_s {
  int size;
  stack_node_t head;
};



static stack_node_t 
stack_node_create(element_t e) 
{
  stack_node_t node = (stack_node_t)malloc(sizeof(*node));
  if (NULL == node) {
    fprintf(stderr, "create stack node failed ...");
    exit(0);
  }

  node->e = e;

  return node;
}

static void 
stack_clear(stack_t s) 
{
  stack_node_t iter;
  while (NULL != s->head) {
    iter = s->head;
    s->head = s->head->next;
    free(iter);
  }
  s->size = 0;
}



stack_t 
stack_create(void) 
{
  stack_t s = (stack_t)malloc(sizeof(*s));
  if (NULL == s) {
    fprintf(stderr, "create stack failed ...");
    exit(0);
  }

  s->size = 0;
  s->head = NULL;

  return s;
}

void 
stack_delete(stack_t* s)
{
  if (NULL != *s) {
    stack_clear(*s);
    free(*s);
    *s = NULL;
  }
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

  return (s->size);
}

void 
stack_push(stack_t s, element_t e) 
{
  stack_node_t new_node;
  ASSERT_STACK(s);

  new_node = stack_node_create(e);
  new_node->next = s->head;
  s->head = new_node;
  ++s->size;
}

element_t 
stack_pop(stack_t s)
{
  stack_node_t top_node;
  element_t top_element;
  ASSERT_STACK(s);
  ASSERT_STACK_ELEMENT(s);

  top_node = s->head;
  top_element = top_node->e;
  s->head = s->head->next;
  free(top_node);
  --s->size;

  return top_element;
}

element_t 
stack_peek(stack_t s)
{
  ASSERT_STACK(s);
  ASSERT_STACK_ELEMENT(s);

  return (s->head->e);
}
