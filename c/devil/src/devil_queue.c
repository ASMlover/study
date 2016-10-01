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
#include <assert.h>
#include <stdio.h>
#include "devil_allocator.h"
#include "devil_queue.h"

typedef struct devil_queue_item_t {
  struct devil_queue_item_t* next;
  void* value;
} devil_queue_item_t;

struct devil_queue_t {
  devil_queue_item_t* head;
  devil_queue_item_t* tail;
  size_t size;
};

devil_queue_t*
devil_queue_create(void)
{
  devil_queue_t* queue = (devil_queue_t*)devil_malloc(sizeof(devil_queue_t));
  assert(NULL != queue);

  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;

  return queue;
}

void
devil_queue_release(devil_queue_t* queue)
{
  devil_queue_item_t* item;
  while (NULL != queue->head) {
    item = queue->head;
    queue->head = queue->head->next;

    devil_free(item);
  }
  devil_free(queue);
}

size_t
devil_queue_size(devil_queue_t* queue)
{
  return queue->size;
}

void
devil_queue_push(devil_queue_t* queue, void* value)
{
  devil_queue_item_t* item
    = (devil_queue_item_t*)devil_malloc(sizeof(devil_queue_item_t));
  item->next = NULL;
  item->value = value;

  if (NULL == queue->head) {
    queue->head = queue->tail = item;
  }
  else {
    queue->tail->next = item;
    queue->tail = item;
  }
  ++queue->size;
}

void*
devil_queue_pop(devil_queue_t* queue)
{
  void* ret;

  if (NULL == queue->head) {
    ret = NULL;
  }
  else {
    devil_queue_item_t* item = queue->head;
    queue->head = queue->head->next;
    ret = item->value;

    devil_free(item);
    --queue->size;
  }

  return ret;
}
