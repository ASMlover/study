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
#include <string.h>
#include "allocator.h"


#if defined(_MSC_VER)
  #define inline  __inline
#endif



enum allocator_size_type {
  ALIGN       = 8, 
  MAX_BYTES   = 16384, 
  NFREELISTS  = MAX_BYTES / ALIGN, 
  MAX_NUMBER  = 64, 
};

typedef struct memory_t {
  struct memory_t* next;
} memory_t;
typedef struct chunk_t {
  struct chunk_t*   next;
  struct memory_t*  data;
} chunk_t;


typedef struct allocator_t {
  memory_t* free_list[NFREELISTS];
  chunk_t*  chunk_list;
} allocator_t;


static allocator_t _s_allocator;


static inline size_t 
free_index(size_t bytes)
{
  return ((bytes + (ALIGN - 1)) / ALIGN - 1);
}

static inline size_t 
min(size_t a, size_t b)
{
  return (a < b ? a : b);
}

static memory_t* 
alloc_chunk(allocator_t* self, size_t index)
{
  size_t alloc_size = (index + 1) * ALIGN;
  size_t chunk_size = 
    min(MAX_BYTES / alloc_size * alloc_size, alloc_size * MAX_NUMBER);

  if (NULL == self->free_list[index]) {
    size_t i;

    self->free_list[index] = (memory_t*)malloc(chunk_size);
    assert(NULL != self->free_list[index]);

    memory_t* node = self->free_list[index];
    for (i = 0; i < chunk_size - alloc_size; i += alloc_size) 
      node = node->next = node + (index + 1) * ALIGN / sizeof(memory_t);
    node->next = NULL;
  }

  return self->free_list[index];
}




void 
allocator_init(void)
{
  memset(_s_allocator.free_list, 0, sizeof(_s_allocator.free_list));
  _s_allocator.chunk_list = NULL;
}

void 
allocator_destroy(void)
{
  memory_t* chunk;
  while (NULL != _s_allocator.chunk_list) {
    chunk = _s_allocator.chunk_list->data;
    _s_allocator.chunk_list = _s_allocator.chunk_list->next;
    free(chunk);
  }
}

void* 
al_malloc(size_t bytes)
{
  /* allocator must be initialized */
  /* bytes must > 0 */

  void* ret;

  assert(bytes > 0);
  if (bytes > MAX_BYTES)
    ret = malloc(bytes);
  else {
    allocator_t* self = &_s_allocator;
    size_t index = free_index(bytes);
    
    if (NULL == self->free_list[index]) {
      memory_t* new_chunk = alloc_chunk(self, index);
      chunk_t*  chunk_node;
      if (free_index(sizeof(chunk_t)) == index) {
        chunk_node = (chunk_t*)self->free_list[index];
        self->free_list[index] = self->free_list[index]->next;
      }
      else 
        chunk_node = (chunk_t*)al_malloc(sizeof(chunk_t));

      chunk_node->data = new_chunk;
      chunk_node->next = self->chunk_list;
      self->chunk_list = chunk_node;
    }

    ret = self->free_list[index];
    self->free_list[index] = self->free_list[index]->next;
  }

  return ret;
}

void 
al_free(void* ptr, size_t bytes)
{
  size_t index;
  memory_t* chunk = (memory_t*)ptr;
  allocator_t* self = &_s_allocator;

  assert(NULL != ptr && bytes > 0);
  index = free_index(bytes);
  chunk->next = self->free_list[index];
  self->free_list[index] = chunk;
}
