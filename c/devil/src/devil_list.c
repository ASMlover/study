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
#include <stdlib.h>
#include "devil_config.h"
#include "devil_allocator.h"
#include "devil_list.h"

typedef struct devil_list_node_t devil_list_node_t;
struct devil_list_node_t {
  devil_list_node_t* prev;
  devil_list_node_t* next;
  void* value;
};

struct devil_list_t {
  devil_list_node_t head;
  size_t size;
};

static inline void
devil_list_insert(devil_list_t* list, devil_list_iter_t pos, void* value)
{
  devil_list_node_t* node = (devil_list_node_t*)devil_malloc(sizeof(devil_list_node_t));
  assert(NULL != node);
  node->value = value;

  node->prev = pos->prev;
  node->next = pos;
  pos->prev->next = node;
  pos->prev = node;

  ++list->size;
}

static inline void*
devil_list_erase(devil_list_t* list, devil_list_iter_t pos)
{
  void* value = NULL;
  devil_list_node_t* prev = pos->prev;
  devil_list_node_t* next = pos->next;

  prev->next = next;
  next->prev = prev;

  value = pos->value;
  devil_free(pos);
  --list->size;

  return value;
}

devil_list_t*
devil_list_create(void)
{
  devil_list_t* list = (devil_list_t*)devil_malloc(sizeof(devil_list_t));
  assert(NULL != list);

  list->head.next = &list->head;
  list->head.prev = &list->head;
  list->size = 0;

  return list;
}

void
devil_list_release(devil_list_t* list)
{
  devil_list_clear(list);
  devil_free(list);
}

int
devil_list_empty(devil_list_t* list)
{
  return (list->head.next == &list->head);
}

size_t
devil_list_size(devil_list_t* list)
{
  return list->size;
}

void
devil_list_clear(devil_list_t* list)
{
  devil_list_node_t* node;
  devil_list_node_t* iter = list->head.next;
  while (iter != &list->head) {
    node = iter;
    iter = iter->next;

    devil_list_erase(list, node);
  }
}

void
devil_list_pushback(devil_list_t* list, void* value)
{
  devil_list_insert(list, &list->head, value);
}

void
devil_list_pushfront(devil_list_t* list, void* value)
{
  devil_list_insert(list, list->head.next, value);
}

void*
devil_list_popback(devil_list_t* list)
{
  if (list->head.next == &list->head) {
    return NULL;
  }
  else {
    devil_list_iter_t pos = (&list->head)->prev;
    return devil_list_erase(list, pos);
  }
}

void*
devil_list_popfront(devil_list_t* list)
{
  if (list->head.next == &list->head)
    return NULL;
  else
    return devil_list_erase(list, list->head.next);
}

void*
devil_list_front(devil_list_t* list)
{
  if (list->head.next == &list->head)
    return NULL;
  else
    return list->head.next->value;
}

void*
devil_list_back(devil_list_t* list)
{
  if (list->head.next == &list->head)
    return NULL;
  else
    return (&list->head)->prev->value;
}

void
devil_list_traverse(devil_list_t* list, void (*visitor)(void*))
{
  devil_list_node_t* node;

  if (NULL == visitor)
    return;

  node = list->head.next;
  while (node != &list->head) {
    visitor(node->value);

    node = node->next;
  }
}

devil_list_iter_t
devil_list_begin(devil_list_t* list)
{
  return list->head.next;
}

devil_list_iter_t
devil_list_end(devil_list_t* list)
{
  return &list->head;
}

devil_list_iter_t
devil_list_iter_next(devil_list_iter_t iter)
{
  return iter->next;
}

devil_list_iter_t
devil_list_iter_prev(devil_list_iter_t iter)
{
  return iter->prev;
}

void*
devil_list_iter_value(devil_list_iter_t iter)
{
  return iter->value;
}
