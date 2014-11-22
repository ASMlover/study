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

typedef int ElementType;

typedef struct Node {
  struct Node* next;
  ElementType value;
} Node;

typedef struct Queue {
  Node* head;
  Node* rear;
} Queue;

static Node* node_create(ElementType value) {
  Node* node = (Node*)malloc(sizeof(*node));

  if (NULL != node) {
    node->next = NULL;
    node->value = value;
  }

  return node;
}

static void queue_clear(Queue* queue) {
  Node* node;

  while (NULL != queue->head) {
    node = queue->head;
    queue->head = queue->head->next;
    free(node);
  }
}

static Queue* queue_init(void) {
  Queue* queue = (Queue*)malloc(sizeof(*queue));

  if (NULL != queue) {
    queue->head = NULL;
    queue->rear = NULL;
  }

  return queue;
}

static void queue_destroy(Queue** queue) {
  if (NULL != *queue) {
    queue_clear(*queue);
    free(*queue);
    *queue = NULL;
  }
}

static int queue_empty(Queue* queue) {
  assert(NULL != queue);

  return (NULL == queue->head);
}

static void queue_push(Queue* queue, ElementType value) {
  assert(NULL != queue);

  {
    Node* node = node_create(value);
    if (NULL == queue->head) {
      queue->head = queue->rear = node;
    }
    else {
      queue->rear->next = node;
      queue->rear = node;
    }
  }
}

static void queue_pop(Queue* queue) {
  assert(NULL != queue);

  {
    Node* node = queue->head;
    if (NULL != node) {
      queue->head = node->next;
      free(node);
    }
  }
}

static ElementType queue_top(Queue* queue) {
  assert(NULL != queue && NULL != queue->head);

  return queue->head->value;
}

void linked_queue(void) {
  Queue* queue = queue_init();
  int i;

  fprintf(stdout, "\n**************LinkedQueue**************\n");

  for (i = 0; i < 10; ++i)
    queue_push(queue, i * i + 343);

  while (!queue_empty(queue)) {
    fprintf(stdout, "LinkedQueue element value is : %d\n", 
        queue_top(queue));
    queue_pop(queue);
  }

  queue_destroy(&queue);
}
