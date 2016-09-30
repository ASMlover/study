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
#include "devil_config.h"
#include "devil_types.h"
#include "devil_spinlock.h"
#include "devil_thread.h"

enum devil_allocator_size_type {
  ALIGN = 8,
  MAX_BYTES = 1024,
  NFREELISTS = MAX_BYTES / ALIGN,
  MAX_NUMBER = 64,
  PREFIX_SIZE = sizeof(size_t),
};
enum devil_allocator_init_type {
  INIT_NO = 0,
  INIT_YES = 1,
};

typedef struct devil_memory_t {
  size_t index;
  struct devil_memory_t* next;
} devil_memory_t;

typedef struct devil_allocator_t {
  devil_memory_t* free_list[NFREELISTS];
  void** chunk_list;
  size_t chunk_count;
  size_t chunk_storage;
  devil_spinlock_t spinlock;
  int initialized;
} devil_allocator_t;

static devil_allocator_t sAllocator;

static inline size_t
freelist_index(size_t bytes)
{
  return ((bytes + (ALIGN - 1)) / ALIGN - 1);
}

static inline void
insert_chunk(devil_allocator_t* self, void* chunk)
{
  if (self->chunk_count == self->chunk_storage) {
    size_t new_chunk_storage = self->chunk_storage + NFREELISTS;
    void** new_chunk_list =
      (void**)malloc(sizeof(void*) * new_chunk_storage);
    assert(NULL != new_chunk_list);

    memmove(new_chunk_list,
        self->chunk_list,
        sizeof(void*) * self->chunk_count);
    free(self->chunk_list);
    self->chunk_list = new_chunk_list;
    self->chunk_storage = new_chunk_storage;
  }

  self->chunk_list[self->chunk_count++] = chunk;
}

static devil_memory_t*
alloc_chunk(devil_allocator_t* self, size_t index)
{
  size_t alloc_size = (index + 1) * ALIGN + PREFIX_SIZE;
  size_t chunk_size = alloc_size * MAX_NUMBER;

  if (NULL == self->free_list[index]) {
    size_t i;
    devil_memory_t* node;

    self->free_list[index] = (devil_memory_t*)malloc(chunk_size);
    assert(NULL != self->free_list[index]);
    insert_chunk(self, self->free_list[index]);

    node = self->free_list[index];
    for (i = 0; i < chunk_size - alloc_size; i += alloc_size) {
      node->index = index;
      node = node->next = (devil_memory_t*)((byte_t*)node + alloc_size);
    }
    node->index = index;
    node->next = NULL;
  }

  return self->free_list[index];
}

void
devil_allocator_init(void)
{
  if (INIT_YES != sAllocator.initialized) {
    memset(sAllocator.free_list, 0, sizeof(sAllocator.free_list));
    sAllocator.chunk_list = (void**)malloc(sizeof(void*) * NFREELISTS);
    assert(NULL != _s_allocator.chunk_list);
    sAllocator.chunk_count = 0;
    sAllocator.chunk_storage = NFREELISTS;
    devil_spinlock_init(&sAllocator.spinlock);

    sAllocator.initialized = INIT_YES;
  }
}

void
devil_allocator_destroy(void)
{
  size_t i;

  if (INIT_YES != sAllocator.initialized)
    return;

  for (i = 0; i < sAllocator.chunk_count; ++i)
    free(sAllocator.chunk_list[i]);
  free(sAllocator.chunk_list);

  devil_spinlock_destroy(&sAllocator.spinlock);
  sAllocator.initialized = INIT_NO;
}

void*
devil_malloc(size_t bytes)
{
  /*
   * The allocator must has been initialized,
   * if not, we will initialize it in sl_malloc.
   *
   * bytes must > 0.
   */

  void* ret;

  assert(bytes > 0);
  if (bytes > MAX_BYTES) {
    ret = malloc(bytes + PREFIX_SIZE);
    *(size_t*)ret = NFREELISTS;
    ret = (byte_t*)ret + PREFIX_SIZE;
  }
  else {
    devil_allocator_t* self = &sAllocator;
    size_t index = freelist_index(bytes);

    if (INIT_YES != self->initialized)
      devil_allocator_init();

    devil_spinlock_lock(&self->spinlock);
    if (NULL == self->free_list[index])
      alloc_chunk(self, index);

    ret = (byte_t*)self->free_list[index] + PREFIX_SIZE;
    self->free_list[index] = self->free_list[index]->next;
    devil_spinlock_unlock(&self->spinlock);
  }

  return ret;
}

void
devil_free(void* ptr)
{
  void* realptr;
  size_t index;
  devil_allocator_t* self = &sAllocator;

  assert(NULL != ptr);
  realptr = (byte_t*)ptr - PREFIX_SIZE;
  index = *(size_t*)realptr;
  if (NFREELISTS == index)
    free(realptr);
  else {
    devil_memory_t* chunk = (devil_memory_t*)realptr;

    devil_spinlock_lock(&self->spinlock);
    chunk->next = self->free_list[index];
    self->free_list[index] = chunk;
    devil_spinlock_unlock(&self->spinlock);
  }
}
