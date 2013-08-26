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
#include "mutex.h"
#include "allocator.h"

#if defined(_MSC_VER)
  #define inline __inline
#endif


typedef unsigned char byte_t;
enum allocator_size_type {
  ALIGN       = 8, 
  MAX_BYTES   = 2048,
  NFREELISTS  = MAX_BYTES / ALIGN, 
  MAX_NUMBER  = 128, 
  PREFIX_SIZE = sizeof(size_t),
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
  mutex_t           mutex;
}; 

static struct allocator_t _s_allocator;



static inline size_t 
freelist_index(size_t bytes)
{
  return ((bytes + (ALIGN - 1)) / ALIGN - 1);
}

static struct memory_t* 
alloc_chunk(struct allocator_t* self, size_t index)
{
  size_t alloc_size = (PREFIX_SIZE + (index + 1) * ALIGN);
  size_t chunk_size = alloc_size * MAX_NUMBER;

  if (NULL == self->free_list[index]) {
    size_t i;
    struct memory_t* node;

    self->free_list[index] = (struct memory_t*)malloc(chunk_size);
    assert(NULL != self->free_list[index]);

    node = self->free_list[index];
    for (i = 0; i < chunk_size - alloc_size; i += alloc_size) {
      node->index = index;
      node = node->next = (struct memory_t*)((byte_t*)node + alloc_size);
    }
    node->index = index;
    node->next = NULL;
  }

  return self->free_list[index];
}



void 
allocator_init(void)
{
  memset(_s_allocator.free_list, 0, sizeof(_s_allocator.free_list));
  _s_allocator.chunk_list = NULL;
  mutex_init(&_s_allocator.mutex);
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

  mutex_destroy(&_s_allocator.mutex);
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
  if (bytes > MAX_BYTES) {
    ret = malloc(bytes + PREFIX_SIZE);
    *(size_t*)ret = NFREELISTS;
    ret = (byte_t*)ret + PREFIX_SIZE;
  }
  else {
    struct allocator_t* self = &_s_allocator;
    size_t index = freelist_index(bytes);

    mutex_lock(&self->mutex);
    if (NULL == self->free_list[index]) {
      struct memory_t* new_chunk = alloc_chunk(self, index);
      struct chunk_t* chunk_node;
      if (freelist_index(sizeof(struct chunk_t)) == index) {
        chunk_node = (struct chunk_t*)((byte_t*)self->free_list[index] 
            + PREFIX_SIZE);
        self->free_list[index] = self->free_list[index]->next;
      }
      else
        chunk_node = (struct chunk_t*)al_malloc(sizeof(struct chunk_t));

      chunk_node->data = new_chunk;
      chunk_node->next = self->chunk_list;
      self->chunk_list = chunk_node;
    }

    ret = (byte_t*)self->free_list[index] + PREFIX_SIZE;
    self->free_list[index] = self->free_list[index]->next;
    mutex_unlock(&self->mutex);
  }

  return ret;
}

void 
al_free(void* ptr)
{
  void* realptr;
  size_t index;
  struct allocator_t* self = &_s_allocator;

  assert(NULL != ptr);
  realptr = (byte_t*)ptr - PREFIX_SIZE;
  index = *(size_t*)realptr;
  if (NFREELISTS == index)
    free(realptr);
  else {
    struct memory_t* chunk = (struct memory_t*)realptr;

    mutex_lock(&self->mutex);
    chunk->next = self->free_list[index];
    self->free_list[index] = chunk;
    mutex_unlock(&self->mutex);
  }
}
