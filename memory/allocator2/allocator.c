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
  #define inline __inline
#endif


typedef unsigned char byte_t;
enum allocator_size_type {
  ALIGN       = 8, 
  MAX_BYTES   = 256,
  NFREELISTS  = MAX_BYTES / ALIGN, 
  MAX_NUMBER  = 64, 
};


struct memory_t {
  size_t            index;
  struct memory_t*  next;
};
struct chunk_t {
  struct chunk_t* next;
  void*           data;
};


struct allocator_t {
  struct memory_t*  free_list[NFREELISTS];
  struct chunk_t*   chunk_list;
}; 

static struct allocator_t _s_allocator;



static inline size_t 
freelist_index(size_t bytes)
{
  return ((bytes + (ALIGN - 1)) / ALIGN - 1);
}

static inline size_t 
_min(size_t a, size_t b)
{
  return (a < b ? a : b);
}




static struct memory_t* 
alloc_chunk(struct allocator_t* self, size_t index)
{
  size_t alloc_size = (sizeof(size_t) + (index + 1) * ALIGN);
  size_t chunk_size = alloc_size * MAX_NUMBER;

  if (NULL == self->free_list[index]) {
    size_t i;
    struct memory_t* node;

    self->free_list[index] = (struct memory_t*)malloc(chunk_size);
    assert(NULL != self->free_list[index]);

    node = self->free_list[index];
    for (i = 0; i < chunk_size - alloc_size; i += alloc_size) {
      node->index = index;
      node = node->next = node + 1;
    }
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
  void* chunk;
  while (NULL != _s_allocator.chunk_list) {
    chunk = _s_allocator.chunk_list->data;
    _s_allocator.chunk_list = _s_allocator.chunk_list->next;
    free(chunk);
  }
}

void* 
al_malloc(size_t bytes)
{
  /*
   * the allocator must has been initialized
   * bytes must > 0
   */
  
  void* ret;

  assert(bytes > 0);
  if (bytes > MAX_BYTES)
    ret = malloc(bytes);
  else {
    struct allocator_t* self = &_s_allocator;
    size_t index = freelist_index(bytes);

    if (NULL == self->free_list[index]) {
      struct memory_t* new_chunk = alloc_chunk(self, index);
      struct chunk_t* chunk_node;
      if (freelist_index(sizeof(struct chunk_t)) == index) {
        chunk_node = (struct chunk_t*)((byte_t*)self->free_list[index] 
            + sizeof(size_t));
        self->free_list[index] = self->free_list[index]->next;
      }
      else
        chunk_node = (struct chunk_t*)al_malloc(sizeof(struct chunk_t));

      chunk_node->data = new_chunk;
      chunk_node->next = self->chunk_list;
      self->chunk_list = chunk_node;
    }

    ret = (byte_t*)self->free_list[index] + sizeof(size_t);
    self->free_list[index] = self->free_list[index]->next;
  }

  return ret;
}

void 
al_free(void* ptr)
{
  struct memory_t* chunk;
  struct allocator_t* self = &_s_allocator;

  assert(NULL != ptr);
  chunk = (struct memory_t*)((byte_t*)ptr - sizeof(size_t));
  chunk->next = self->free_list[chunk->index];
  self->free_list[chunk->index] = chunk;
}
