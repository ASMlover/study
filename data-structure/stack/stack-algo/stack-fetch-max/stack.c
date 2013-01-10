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
#include "stack.h"


typedef struct stack_node_t {
  element_t val;
  element_t max;
  element_t min;
  struct stack_node_t* next;
} *stack_node_t;

struct stack_t {
  int size;
  stack_node_t head;
};


static stack_node_t 
stack_node_create(element_t val) 
{
  stack_node_t node;

  node = (stack_node_t)malloc(sizeof(*node));
  assert(NULL != node);

  node->val = val;
  node->min = node->max = 0;

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
    free(node);
  }
  s->size = 0;
}



stack_t 
stack_create(void)
{
  stack_t s;
  s = (stack_t)malloc(sizeof(*s));
  assert(NULL != s);

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
  assert(NULL != s);
  return (NULL ==  s->head);
}

int 
stack_size(stack_t s)
{
  assert(NULL != s);
  return s->size;
}

void 
stack_push(stack_t s, element_t val)
{
  stack_node_t node;

  assert(NULL != s);
  node = stack_node_create(val);
  if (NULL == s->head) {
    node->max = val;
    node->min = val;
  } 
  else {
    node->max = (val > s->head->max ? val : s->head->max);
    node->min = (val < s->head->min ? val : s->head->min);
  }
  node->next = s->head;
  s->head = node;
  ++s->size;
}

element_t 
stack_pop(stack_t s)
{
  stack_node_t node;
  element_t    pop_elem;
  assert(NULL != s);
  assert(NULL != s->head);

  node = s->head;
  s->head = s->head->next;
  pop_elem = node->val;
  free(node);
  --s->size;

  return pop_elem;
}

element_t 
stack_peek(stack_t s)
{
  assert(NULL != s);
  assert(NULL != s->head);

  return s->head->val;
}

element_t 
stack_max(stack_t s)
{
  assert(NULL != s);
  assert(NULL != s->head);

  return s->head->max;
}

element_t 
stack_min(stack_t s)
{
  assert(NULL != s);
  assert(NULL != s->head);

  return s->head->min;
}
