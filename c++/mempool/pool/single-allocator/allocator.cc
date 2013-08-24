//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "allocator.h"



struct memory_t {
  memory_t* next;
};

struct chunk_t {
  chunk_t*  next;
  memory_t* data;
};



static inline size_t 
freelist_index(size_t bytes)
{
  return ((bytes + (allocator_t::ALIGN - 1)) / allocator_t::ALIGN - 1);
}

static inline size_t 
min(size_t a, size_t b)
{
  return (a < b ? a : b);
}


memory_t* 
allocator_t::alloc_chunk(size_t index)
{
  size_t alloc_size = (index + 1) * ALIGN;
  size_t chunk_size = 
    min(MAX_BYTES / alloc_size * alloc_size, alloc_size * MAX_NUMBER);
  
  if (NULL == free_list_[index]) {
    free_list_[index] = (memory_t*)malloc(chunk_size);
    assert(NULL != free_list_[index]);

    memory_t* node = free_list_[index];
    for (size_t i = 0; i < chunk_size - alloc_size; i += alloc_size) 
      node = node->next = node + (index + 1) * ALIGN / sizeof(*node);
    node->next = NULL;
  }

  return free_list_[index];
}

allocator_t::allocator_t(void)
{
  memset(free_list_, 0, sizeof(free_list_));
  chunk_list_ = NULL;
}

allocator_t::~allocator_t(void)
{
  memory_t* chunk;
  while (NULL != chunk_list_) {
    chunk = chunk_list_->data;
    chunk_list_ = chunk_list_->next;
    free(chunk);
  }
}

allocator_t& 
allocator_t::singleton(void)
{
  static allocator_t s;
  return s;
}

void* 
allocator_t::alloc(size_t size)
{
  assert(size > 0);

  void* ret;

  if (size > MAX_BYTES)
    ret = malloc(size);
  else {
    size_t index = freelist_index(size);
    if (NULL == free_list_[index]) {
      memory_t* new_chunk = alloc_chunk(index);
      chunk_t*  chunk_node;
      if (freelist_index(sizeof(chunk_t)) == index) {
        chunk_node = (chunk_t*)free_list_[index];
        free_list_[index] = free_list_[index]->next;
      }
      else 
        chunk_node = (chunk_t*)alloc(sizeof(chunk_t));

      chunk_node->data = new_chunk;
      chunk_node->next = chunk_list_;
      chunk_list_ = chunk_node;
    }

    ret = free_list_[index];
    free_list_[index] = free_list_[index]->next;
  }

  return ret;
}

void 
allocator_t::dealloc(void* ptr, size_t size)
{
  assert(NULL != ptr && size > 0);

  size_t index = freelist_index(size);
  memory_t* free_block = (memory_t*)ptr;
  free_block->next = free_list_[index];
  free_list_[index] = free_block;
}
