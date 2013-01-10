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


struct deque_s {
  int limit;
  int front;
  int rear;
  int size;
  element_t* elements;
};

#define DEQUE_LIMIT_DEF   (8)
#define ASSERT_DEQUE(q) {\
  if (NULL == (q)) {\
    fprintf(stderr, "deque invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_DEQUE_UNDERFLOW(q) {\
  if (0 == (q)->size) {\
    fprintf(stderr, "deque underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_DEQUE_OVERFLOW(q) {\
  if ((q)->size == (q)->limit) {\
    fprintf(stderr, "deque overflow ...");\
    exit(0);\
  }\
}



deque_t 
deque_create(int limit) 
{
  deque_t q = (deque_t)malloc(sizeof(*q));
  ASSERT_DEQUE(q);

  q->limit = (limit < DEQUE_LIMIT_DEF ? DEQUE_LIMIT_DEF : limit);
  q->front = q->rear = 0;
  q->size  = 0;
  q->elements = (element_t*)malloc(q->limit * sizeof(element_t));
  if (NULL == q->elements) {
    fprintf(stderr, "create deque failed ...");
    exit(0);
  }

  return q;
}

void 
deque_delete(deque_t* q) 
{
  if (NULL != *q) {
    if (NULL != (*q)->elements) 
      free((*q)->elements);
    free(*q);
    *q = NULL;
  }
}

int 
deque_empty(deque_t q) 
{
  ASSERT_DEQUE(q);

  return (0 == q->size);
}

int 
deque_size(deque_t q) 
{
  ASSERT_DEQUE(q);

  return (q->size);
}

void 
deque_push_front(deque_t q, element_t e)
{
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_OVERFLOW(q);

  if (0 != q->size)
    q->front = (q->front - 1 + q->limit) % q->limit;
  else
    q->rear = (q->rear + 1) % q->limit;
  q->elements[q->front] = e;
  ++q->size;
}

void 
deque_push_rear(deque_t q, element_t e)
{
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_OVERFLOW(q);

  q->elements[q->rear] = e;
  q->rear = (q->rear + 1) % q->limit;
  ++q->size;
}

element_t 
deque_pop_front(deque_t q) 
{
  element_t e;
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_UNDERFLOW(q);

  e = q->elements[q->front];
  q->front = (q->front + 1) % q->limit;
  --q->size;

  return e;
}

element_t 
deque_pop_rear(deque_t q)
{
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_UNDERFLOW(q);

  q->rear = (q->rear - 1 + q->limit) % q->limit;
  --q->size;

  return q->elements[q->rear];
}

element_t 
deque_peek_front(deque_t q)
{
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_UNDERFLOW(q);

  return q->elements[q->front];
}

element_t 
deque_peek_rear(deque_t q) 
{
  int rear;
  ASSERT_DEQUE(q);
  ASSERT_DEQUE_UNDERFLOW(q);

  rear = (q->rear - 1 + q->limit) % q->limit;
  return q->elements[rear];
}
