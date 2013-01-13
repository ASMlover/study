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
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
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
#include "list.h"


typedef struct list_node_s {
  struct list_node_s* next;
  element_t e;
} *list_node_t;

struct list_s {
  int size;
  list_node_t front;
  list_node_t rear;
};

#define ASSERT_LIST(l) {\
  if (NULL == (l)) {\
    fprintf(stderr, "circular list invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_UNDERFLOW(l) {\
  if (NULL == (l)->front) {\
    fprintf(stderr, "circular list underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_LIST_NODE(n) {\
  if (NULL == (n)) {\
    fprintf(stderr, "create circular list node failed ...");\
    exit(0);\
  }\
}




static list_node_t 
list_node_create(element_t e) 
{
  list_node_t new_node = (list_node_t)malloc(sizeof(*new_node));
  ASSERT_LIST_NODE(new_node);

  new_node->e = e;

  return new_node;
}

static void 
list_clear(list_t l) 
{
  if (0 != l->size) {
    list_node_t iter = l->front;
    list_node_t node;

    do {
      node = iter;
      iter = iter->next;
      free(node);
    } while (iter != l->front);

    l->size = 0;
  }
}



list_t 
list_create(void)
{
  list_t l = (list_t)malloc(sizeof(*l));
  ASSERT_LIST(l);

  l->size = 0;
  l->front = l->rear = NULL;

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

  return l->size;
}

void 
list_append(list_t l, element_t e) 
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
  l->rear->next = l->front;
  ++l->size;
}

element_t 
list_remove(list_t l) 
{
  element_t e;
  list_node_t old_node;
  ASSERT_LIST(l);
  ASSERT_UNDERFLOW(l);

  old_node = l->front;
  l->front = l->front->next;
  e = old_node->e;
  free(old_node);
  if (0 == --l->size)
    l->front = NULL;

  return e;
}

void 
list_traverse(list_t l, void (*visit)(element_t)) 
{
  list_node_t iter;
  ASSERT_LIST(l);
  ASSERT_UNDERFLOW(l);

  iter = l->front;
  do {
    visit(iter->e);
    iter = iter->next;
  } while (iter != l->front);
}
