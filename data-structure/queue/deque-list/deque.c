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
#include "deque.h"


typedef struct deque_node_s {
  element_t e;
  struct deque_node_s* prev;
  struct deque_node_s* next;
} *deque_node_t;

struct deque_s {
  int size;
  struct deque_node_s head;
};


#define ASSERT_DEQUE(q) {\
  if (NULL == (q)) {\
    fprintf(stderr, "deque invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_DEQUE_NODE(n) {\
  if (NULL == (n)) {\
    fprintf(stderr, "deque node invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_DEQUE_UNDERFLOW(q) {\
  if ((q)->head.next == &(q)->head) {\
    fprintf(stderr, "deque underflow ...");\
    exit(0);\
  }\
}

static void  
SHOW_DEQUE_NODE_ADDRESS(const char* s, void* n) 
{
#ifdef DEBUG
  fprintf(stdout, "\t===> [%s] deque node address : 0x%p\n", (s), (n));
#endif
}



static deque_node_t 
deque_node_create(element_t e)
{
  deque_node_t new_node = (deque_node_t)malloc(sizeof(*new_node));
  ASSERT_DEQUE_NODE(new_node);

  new_node->e = e;
  new_node->prev = NULL;
  new_node->next = NULL;

  return new_node;
}

static void 
deque_insert(deque_t q, deque_node_t pos, element_t e)
{
  deque_node_t new_node = deque_node_create(e);
  
  new_node->prev = pos->prev;
  new_node->next = pos;
  pos->prev->next = new_node;
  pos->prev = new_node;

  SHOW_DEQUE_NODE_ADDRESS("enqueue new node", new_node);

  ++q->size;
}

static element_t 
deque_erase(deque_t q, deque_node_t pos) 
{
  deque_node_t prev, next;
  element_t e;

  prev = pos->prev;
  next = pos->next;
  prev->next = next;
  next->prev = prev;
  e = pos->e;
  SHOW_DEQUE_NODE_ADDRESS("dequeue old node", pos);
  free(pos);
  --q->size;

  return e;
}

static void 
deque_clear(deque_t q) 
{
  deque_node_t iter = q->head.next;
  deque_node_t end  = &q->head;
  deque_node_t node;

  while (iter != end) {
    node = iter;
    iter = iter->next;
    deque_erase(q, node);
  }

  q->size = 0;
}



deque_t 
deque_create(void)
{
  deque_t q = (deque_t)malloc(sizeof(*q));
  ASSERT_DEQUE(q);

  q->size = 0;
  q->head.prev = &q->head;
  q->head.next = &q->head;

  return q;
}

void 
deque_delete(deque_t* q) 
{
  if (NULL != *q) {
    deque_clear(*q);
    free(*q);
    *q = NULL;
  }
}

int 
deque_empty(deque_t q) 
{
  ASSERT_DEQUE(q);

  return (q->head.next == &q->head);
}

int 
deque_size(deque_t q) 
{
  ASSERT_DEQUE(q);

  return q->size;
}

void 
deque_push_front(deque_t q, element_t e) 
{
  ASSERT_DEQUE(q);

  deque_insert(q, q->head.next, e);
}

void 
deque_push_rear(deque_t q, element_t e)
{
  ASSERT_DEQUE(q);

  deque_insert(q, &q->head, e);
}

element_t 
deque_pop_front(deque_t q)
{
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_UNDERFLOW(q);

  return deque_erase(q, q->head.next);
}

element_t 
deque_pop_rear(deque_t q) 
{
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_UNDERFLOW(q);

  return deque_erase(q, (&q->head)->prev);
}

element_t 
deque_peek_front(deque_t q) 
{
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_UNDERFLOW(q);

  return q->head.next->e;
}

element_t 
deque_peek_rear(deque_t q) 
{
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_UNDERFLOW(q);

  return (&q->head)->prev->e;
}
