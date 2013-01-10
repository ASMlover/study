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


typedef struct queue_node_s {
  element_t e;
  struct queue_node_s* next;
} *queue_node_t;

struct queue_s {
  int size;
  queue_node_t front;
  queue_node_t rear;
};

#define ASSERT_QUEUE(q) {\
  if (NULL == (q)) {\
    fprintf(stderr, "queue invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_QUEUE_ELEMENT(n) {\
  if (NULL == (n)) {\
    fprintf(stderr, "queue element invalid ...");\
    exit(0);\
  }\
}
#define ASSERT_QUEUE_UNDERFLOW(q) {\
  if (NULL == (q)->front) {\
    fprintf(stderr, "queue underflow ...");\
    exit(0);\
  }\
}



static queue_node_t 
queue_node_create(element_t e)
{
  queue_node_t new_node = (queue_node_t)malloc(sizeof(*new_node));
  ASSERT_QUEUE_ELEMENT(new_node);

  new_node->e = e;
  new_node->next = NULL;

  return new_node;
}

static void 
queue_clear(queue_t q) 
{
  queue_node_t iter;

  while (NULL != q->front) {
    iter = q->front;
    q->front = q->front->next;
    free(iter);
  }

  q->size = 0;
}




queue_t 
queue_create(void)
{
  queue_t q = (queue_t)malloc(sizeof(*q));
  ASSERT_QUEUE(q);

  q->size = 0;
  q->front = q->rear = NULL;

  return q;
}

void 
queue_delete(queue_t* q) 
{
  if (NULL != *q) {
    queue_clear(*q);
    free(*q);
    *q = NULL;
  }
}

int 
queue_empty(queue_t q) 
{
  ASSERT_QUEUE(q);

  return (NULL == q->front);
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
  queue_node_t new_node;
  ASSERT_QUEUE(q);

  new_node = queue_node_create(e);
  if (NULL == q->front)
    q->front = q->rear = new_node;
  else {
    q->rear->next = new_node;
    q->rear = new_node;
  }
  ++q->size;
}

element_t 
queue_dequeue(queue_t q) 
{
  element_t e;
  queue_node_t old_node;
  ASSERT_QUEUE(q);
  ASSERT_QUEUE_UNDERFLOW(q);

  old_node = q->front;
  q->front = q->front->next;
  e = old_node->e;
  free(old_node);
  --q->size;

  return e;
}

element_t 
queue_peek(queue_t q) 
{
  ASSERT_QUEUE(q);
  ASSERT_QUEUE_UNDERFLOW(q);

  return q->front->e;
}
