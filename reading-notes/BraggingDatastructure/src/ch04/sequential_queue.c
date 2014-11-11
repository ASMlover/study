/*
 * Copyright (c) 2014 ASMlover. All rights reserved.
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

#define DEF_STORAGE (128)

typedef int ElementType;

typedef struct Queue {
  int front;
  int rear;
  ElementType elements[DEF_STORAGE];
} Queue;


static Queue* queue_init(void) {
  Queue* queue = (Queue*)malloc(sizeof(*queue));

  if (NULL != queue) {
    queue->front = 0;
    queue->rear = 0;
  }

  return queue;
}

static void queue_destroy(Queue** queue) {
  if (NULL != *queue) {
    free(*queue);
    *queue = NULL;
  }
}

static int queue_empty(Queue* queue) {
  assert(NULL != queue);

  return (queue->front == queue->rear);
}

static void queue_push(Queue* queue, ElementType value) {
  if (NULL != queue && queue->rear < DEF_STORAGE)
    queue->elements[queue->rear++] = value;
}

static void queue_pop(Queue* queue) {
  if (NULL != queue && queue->front < queue->rear)
    ++queue->front;
}

static ElementType queue_top(Queue* queue) {
  assert(NULL != queue && queue->front < queue->rear);

  return queue->elements[queue->front];
}


void sequential_queue(void) {
  Queue* queue = queue_init();
  int i;

  fprintf(stdout, "\n**************SequentialQueue**************\n");

  for (i = 0; i < 10; ++i)
    queue_push(queue, i * i + 343);

  while (!queue_empty(queue)) {
    fprintf(stdout, "SequentialQueue element value is : %d\n", 
        queue_top(queue));
    queue_pop(queue);
  }

  queue_destroy(&queue);
}
