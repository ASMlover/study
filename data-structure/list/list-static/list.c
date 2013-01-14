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
  int next;
  element_t e;
} list_node_t;

struct list_s {
  int capacity;
  int size;
  int front;
  int rear;
  int free;
  list_node_t* elements;
};


#define LIST_CAPACITY_DEF (8)
#define ASSERT_LIST(l) {\
  if (NULL == (l)) {\
    fprintf(stderr, "static list invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_LIST_UNDERFLOW(l) {\
  if (0 == (l)->size) {\
    fprintf(stderr, "static list underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_LIST_OVERFLOW(l) {\
  if ((l)->capacity == (l)->size) {\
    fprintf(stderr, "static list overflow ...");\
    exit(0);\
  }\
}


static void 
list_initialize(list_t l) 
{
  int i;
  for (i = 0; i < l->capacity - 1; ++i)
    l->elements[i].next = i + 1;
  l->elements[l->capacity - 1].next = -1;
}




list_t 
list_create(int capacity) 
{
  list_t l = (list_t)malloc(sizeof(*l));
  ASSERT_LIST(l);

  l->capacity = (capacity < LIST_CAPACITY_DEF ? 
    LIST_CAPACITY_DEF : capacity);
  l->size     = 0;
  l->front = l->rear = 0;
  l->free     = 0;
  l->elements = (element_t)malloc(l->capacity * sizeof(list_node_t));
  if (NULL == l->elements) {
    fprintf(stderr, "create static list failed ...");
    exit(0);
  }
  list_initialize(l);

  return l;
}

void 
list_delete(list_t* l) 
{
  if (NULL != *l) {
    if (NULL != (*l)->elements) 
      free((*l)->elements);
    free(*l);
    *l = NULL;
  }
}

int 
list_empty(list_t l) 
{
  ASSERT_LIST(l);

  return (0 == l->size);
}

int 
list_size(list_t l) 
{
  ASSERT_LIST(l);

  return l->size;
}

void 
list_push_front(list_t l, element_t e)
{
  int i;
  ASSERT_LIST(l);
  ASSERT_LIST_OVERFLOW(l);

  i = l->free;
  l->free = l->elements[i].next;
  l->elements[i].e = e;
  if (0 != l->size)
    l->elements[i].next = l->front;
  else 
    l->rear = i;
  l->front = i;
  ++l->size;
}

void 
list_push_back(list_t l, element_t e) 
{
  int i;
  ASSERT_LIST(l);
  ASSERT_LIST_OVERFLOW(l);

  i = l->free;
  l->free = l->elements[i].next;
  l->elements[i].e = e;
  l->rear = i;
  ++l->size;
}

element_t 
list_pop_front(list_t l)
{
  int i;
  element_t e;
  ASSERT_LIST(l);
  ASSERT_LIST_UNDERFLOW(l);

  i = l->front;
  e = l->elements[i].e;
  l->front = l->elements[i].next;
  l->elements[i].next = l->free;
  l->free = i;
  if (0 == --l->size) 
    l->front = l->rear = l->free = 0;

  return e;
}
