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
#include "sl_allocator.h"
#include "sl_list.h"


#if defined(_WINDOWS_) || defined(_MSC_VER)
  #define inline __inline
#endif



typedef struct sl_list_node_t sl_list_node_t;
struct sl_list_node_t {
  sl_list_node_t* prev;
  sl_list_node_t* next;
  void* value;
};

struct sl_list_t {
  sl_list_node_t head;
  size_t size;
};



static inline void 
sl_list_insert(sl_list_t* list, sl_list_iter_t* pos, void* value)
{
  sl_list_node_t* node = (sl_list_node_t*)sl_malloc(sizeof(sl_list_node_t));
  assert(NULL != node);
  node->value = value;

  node->prev = pos->prev;
  node->next = pos;
  pos->prev->next = node;
  pos->prev = node;

  ++list->size;
}

static inline void* 
sl_list_erase(sl_list_t* list, sl_list_iter_t* pos)
{
  void* value = NULL;
  sl_list_node_t* prev = pos->prev;
  sl_list_node_t* next = pos->next;

  prev->next = next;
  next->prev = prev;
  
  value = pos->value;
  sl_free(pos);
  --list->size;

  return value;
}






sl_list_t* 
sl_list_create(void)
{
  sl_list_t* list = (sl_list_t*)sl_malloc(sizeof(sl_list_t));
  assert(NULL != list);

  list->head.next = &list->head;
  list->head.prev = &list->head;
  list->size = 0;

  return list;
}

void 
sl_list_release(sl_list_t* list)
{
  sl_list_clear(list);
  sl_free(list);
}

int 
sl_list_empty(sl_list_t* list)
{
  return (list->head.next == &list->head);
}

size_t 
sl_list_size(sl_list_t* list)
{
  return list->size;
}

void 
sl_list_clear(sl_list_t* list)
{
  sl_list_node_t* node;
  sl_list_node_t* iter = list->head.next;
  while (iter != &list->head) {
    node = iter;
    iter = iter->next;

    sl_list_erase(list, node);
  }
}

void 
sl_list_pushback(sl_list_t* list, void* value)
{
  sl_list_insert(list, &list->head, value);
}

void 
sl_list_pushfront(sl_list_t* list, void* value)
{
  sl_list_insert(list, list->head.next, value);
}

void* 
sl_list_popback(sl_list_t* list)
{
  if (list->head.next == &list->head)
    return NULL;
  else {
    sl_list_iter_t* pos = (&list->head)->prev;
    return sl_list_erase(list, pos);
  }
}

void* 
sl_list_popfront(sl_list_t* list)
{
  if (list->head.next == &list->head)
    return NULL;
  else 
    return sl_list_erase(list, list->head.next);
}

void* 
sl_list_front(sl_list_t* list)
{
  if (list->head.next == &list->head)
    return NULL;
  else 
    return list->head.next->value;
}

void* 
sl_list_back(sl_list_t* list)
{
  if (list->head.next == &list->head)
    return NULL;
  else 
    return (&list->head)->prev->value;
}

void 
sl_list_traverse(sl_list_t* list, void (*visit)(void*))
{
  sl_list_node_t* node;

  if (NULL == visit)
    return;

  node = list->head.next;
  while (node != &list->head) {
    visit(node->value);

    node = node->next;
  }
}

sl_list_iter_t* 
sl_list_begin(sl_list_t* list)
{
  return list->head.next;
}

sl_list_iter_t* 
sl_list_end(sl_list_t* list)
{
  return &list->head;
}

sl_list_iter_t* 
sl_list_iter_next(sl_list_iter_t* iter)
{
  return iter->next;
}

sl_list_iter_t* 
sl_list_iter_prev(sl_list_iter_t* iter)
{
  return iter->prev;
}

void* 
sl_list_iter_value(sl_list_iter_t* iter)
{
  return iter->value;
}
