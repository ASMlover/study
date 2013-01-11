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
#include "list.h"


typedef struct list_node_s {
  struct list_node_s* next;
  element_t e;
} *list_node_t;

struct list_s {
  list_node_t front;
  list_node_t rear;
  int size;
};


#define ASSERT_LIST(l) {\
  if (NULL == (l)) {\
    fprintf(stderr, "list invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_LIST_UNDERFLOW(l) {\
  if (NULL == (l)->front) {\
    fprintf(stderr, "list underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_LIST_CHECK(l, i) {\
  if ((i) < 0 || (i) >= (l)->size) {\
    fprintf(stderr, "list out of index ...");\
    exit(0);\
  }\
}



static list_node_t 
list_node_create(element_t e) 
{
  list_node_t new_node = (list_node_t)malloc(sizeof(*new_node));
  if (NULL == new_node) {
    fprintf(stderr, "create list node failed ...");
    exit(0);
  }

  new_node->next = NULL;
  new_node->e = e;

  return new_node;
}



list_t 
list_create(void) 
{
  list_t l = (list_t)malloc(sizeof(*l));
  ASSERT_LIST(l);

  l->front = l->rear = NULL;
  l->size = 0;
  
  return l;
}

void 
list_delete(list_t* l) 
{
  if (NULL != *l) {
    list_clear(*l);
    free(*l);
    *l = NULL;
  }
}

int 
list_empty(list_t l) 
{
  ASSERT_LIST(l);

  return (NULL == l->front);
}

int 
list_size(list_t l) 
{
  ASSERT_LIST(l);

  return (l->size);
}

void 
list_clear(list_t l) 
{
  list_node_t iter;
  while (NULL != l->front) {
    iter = l->front;
    l->front = l->front->next;
    free(iter);
  }
  l->size = 0;
}

void 
list_insert(list_t l, element_t e)
{
  list_node_t new_node;
  ASSERT_LIST(l);

  new_node = list_node_create(e);
  if (NULL == l->front)
    l->front = l->rear = new_node;
  else {
    l->rear->next = new_node;
    l->rear = new_node;
  }
  ++l->size;
}

element_t 
list_remove(list_t l)
{
  element_t e;
  list_node_t old_node;
  ASSERT_LIST(l);
  ASSERT_LIST_UNDERFLOW(l);

  old_node = l->front;
  l->front = l->front->next;
  e = old_node->e;
  free(old_node);
  --l->size;

  return e;
}

int 
list_find(list_t l, element_t e, 
  int (*cmp)(element_t, element_t, int), int elem_sz)
{
  int i = 0;
  list_node_t iter;
  ASSERT_LIST(l);
  ASSERT_LIST_UNDERFLOW(l);

  iter = l->front;
  while (NULL != iter) {
    if (cmp(e, iter->e, elem_sz))
      return i;
    ++i;
  }

  return (-1);
}

void 
list_traverse(list_t l, void (*visit)(element_t)) 
{
  list_node_t iter;
  ASSERT_LIST(l);
  ASSERT_LIST_UNDERFLOW(l);

  iter = l->front;
  while (NULL != iter) {
    visit(iter->e);
    iter = iter->next;
  }
}
