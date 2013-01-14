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
#include "list.h"


typedef struct list_node_s {
  struct list_node_s* prev;
  struct list_node_s* next;
  element_t e;
} *list_node_t;

struct list_s {
  int size;
  struct list_node_s head;
};


#define ASSERT_LIST(l) {\
  if (NULL == (l)) {\
    fprintf(stderr, "circular list invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_LIST_NODE(n) {\
  if (NULL == (n)) {\
    fprintf(stderr, "circular list node invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_LIST_UNDERFLOW(l) {\
  if ((l)->head.next == &(l)->head) {\
    fprintf(stderr, "circular list underflow ...");\
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
list_insert(list_t l, list_node_t pos, element_t e)
{
  list_node_t new_node;

  new_node = list_node_create(e);
  new_node->prev = pos->prev;
  new_node->next = pos;
  pos->prev->next = new_node;
  pos->prev = new_node;
  ++l->size;
}

static element_t 
list_erase(list_t l, list_node_t pos)
{
  list_node_t prev, next;
  element_t e;

  prev = pos->prev;
  next = pos->next;
  prev->next = next;
  next->prev = prev;
  e = pos->e;
  free(pos);
  --l->size;

  return e;
}




list_t 
list_create(void) 
{
  list_t l = (list_t)malloc(sizeof(*l));
  ASSERT_LIST(l);

  l->size = 0;
  l->head.prev = l->head.next = &l->head;

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

  return (l->head.next == &l->head);
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
  list_node_t iter, node;
  ASSERT_LIST(l);

  iter = l->head.next;
  while (iter != &l->head) {
    node = iter;
    iter = iter->next;
    list_erase(l, iter);
  }
  l->size = 0;
}

void 
list_push_front(list_t l, element_t e)
{
  ASSERT_LIST(l);

  list_insert(l, &l->head, e);
}

void 
list_push_back(list_t l, element_t e)
{
  ASSERT_LIST(l);
  
  list_insert(l, l->head.next, e);
}

element_t 
list_pop_front(list_t l)
{
  ASSERT_LIST(l);
  ASSERT_LIST_UNDERFLOW(l);

  return list_erase(l, (&l->head)->prev);
}

element_t 
list_pop_back(list_t l) 
{
  ASSERT_LIST(l);
  ASSERT_LIST_UNDERFLOW(l);

  return list_erase(l, l->head.next);
}

void 
list_circular(list_t l, int circular)
{
  int i;
  list_node_t iter;
  ASSERT_LIST(l);

  iter = l->head.next;
  for (i = 0; i < circular; ++i) {
    fprintf(stdout, "The [%d] circular circular-list show:\n", i + 1);
    while (iter != &l->head) {
      fprintf(stdout, "\tthe element is {value=>%lf}\n", *(double*)iter->e);
      iter = iter->next;
    }
    iter = iter->next;
  }
}
