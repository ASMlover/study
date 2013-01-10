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
#include "queue.h"


struct queue_s {
  int limit;
  int front;
  int rear;
  int size;
  element_t* elements;
};

#define QUEUE_LIMIT_DEF (8)
#define ASSERT_QUEUE(q) {\
  if (NULL == (q)) {\
    fprintf(stderr, "queue invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_QUEUE_UNDERFLOW(q) {\
  if (0 == (q)->size) {\
    fprintf(stderr, "queue underflow ...");\
    exit(0);\
  }\
}
#define ASSERT_QUEUE_OVERFLOW(q) {\
  if ((q)->size == (q)->limit) {\
    fprintf(stderr, "queue overflow ...");\
    exit(0);\
  }\
}



queue_t 
queue_create(int limit) 
{
  queue_t q = (queue_t)malloc(sizeof(*q));
  ASSERT_QUEUE(q);

  q->limit = (limit < QUEUE_LIMIT_DEF ? QUEUE_LIMIT_DEF : limit);
  q->front = q->rear = 0;
  q->rear  = 0;
  q->elements = (element_t*)malloc(q->limit * sizeof(element_t));
  if (NULL == q->elements) {
    fprintf(stderr, "create queue failed ...");
    exit(0);
  }

  return q;
}

void 
queue_delete(queue_t* q)
{
  if (NULL != *q) {
    if (NULL != (*q)->elements) 
      free((*q)->elements);
    free(*q);
    *q = NULL;
  }
}

int 
queue_empty(queue_t q)
{
  ASSERT_QUEUE(q);

  return (0 == q->size);
}

int 
queue_size(queue_t q)
{
  ASSERT_QUEUE(q);

  return q->size;
}

void 
queue_enqueue(queue_t q, element_t e)
{
  ASSERT_QUEUE(q);
  ASSERT_QUEUE_OVERFLOW(q);

  q->elements[q->rear] = e;
  q->rear = (q->rear + 1) % q->limit;
  ++q->size;
}

element_t 
queue_dequeue(queue_t q)
{
  element_t e;

  ASSERT_QUEUE(q);
  ASSERT_QUEUE_UNDERFLOW(q);

  e = q->elements[q->front];
  q->front = (q->front + 1) % q->limit;
  -- q->size;

  return e;
}
