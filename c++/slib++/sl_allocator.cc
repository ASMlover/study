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
#include "sl_spinlock.h"
#include "sl_allocator.h"


namespace sl {

typedef unsigned char byte_t;
struct memory_t {
  size_t index;
  memory_t* next;
};

class allocator_guard_t : noncopyable {
  spinlock_t* spinlock_;
public:
  allocator_guard_t(spinlock_t* spinlock)
    : spinlock_(spinlock)
  {
    if (NULL != spinlock_)
      spinlock_->lock();
  }

  ~allocator_guard_t(void)
  {
    spinlock_->unlock();
  }
};


memory_t* 
allocator_t::alloc_chunk(size_t index)
{
  size_t alloc_size = (index + 1) * ALIGN + PREFIX_SIZE;
  size_t chunk_size = alloc_size * MAX_NUMBER;

  if (NULL == free_list_[index]) {
    free_list_[index] = (memory_t*)malloc(chunk_size);
    assert(NULL != free_list_[index]);
    insert_chunk(free_list_[index]);

    memory_t* node = free_list_[index];
    for (size_t i = 0; i < chunk_size - alloc_size; i += alloc_size) {
      node->index = index;
      node = node->next = (memory_t*)((byte_t*)node + alloc_size);
    }
    node->index = index;
    node->next = NULL;
  }

  return free_list_[index];
}

void 
allocator_t::insert_chunk(void* chunk)
{
  if (chunk_count_ == chunk_storage_) {
    size_t new_chunk_storage = chunk_storage_ + NFREELISTS;
    void** new_chunk_list = 
      (void**)malloc(sizeof(void*) * new_chunk_storage);
    assert(NULL != new_chunk_list);

    memmove(new_chunk_list, 
        chunk_list_, 
        sizeof(void*) * chunk_storage_);
    free(chunk_list_);
    chunk_list_ = new_chunk_list;
    chunk_storage_ = new_chunk_storage;
  }

  chunk_list_[chunk_count_++] = chunk;
}

allocator_t::allocator_t(void)
{
  memset(free_list_, 0, sizeof(free_list_));
  chunk_list_ = (void**)malloc(sizeof(void*) * NFREELISTS);
  assert(NULL != chunk_list_);
  chunk_count_ = 0;
  chunk_storage_ = NFREELISTS;

  spinlock_ = new spinlock_t();
  assert(NULL != spinlock_);
}

allocator_t::~allocator_t(void)
{
  for (size_t i = 0; i < chunk_count_; ++i) 
    free(chunk_list_[i]);
  free(chunk_list_);

  delete spinlock_;
}

void* 
allocator_t::alloc(size_t bytes)
{
  //! allocator must has been initialized
  //! bytes must > 0 

  assert(bytes > 0);
  
  void* ret;
  if (bytes > MAX_BYTES) {
    ret = malloc(bytes + PREFIX_SIZE);
    *(size_t*)ret = NFREELISTS;
    ret = (byte_t*)ret + PREFIX_SIZE;
  }
  else {
    size_t index = freelist_index(bytes);

    allocator_guard_t lock(spinlock_);
    if (NULL == free_list_[index])
      alloc_chunk(index);

    ret = (byte_t*)free_list_[index] + PREFIX_SIZE;
    free_list_[index] = free_list_[index]->next;
  }

  return ret;
}

void 
allocator_t::dealloc(void* ptr)
{
  assert(NULL != ptr);

  void* realptr = (byte_t*)ptr - PREFIX_SIZE;
  size_t index = *(size_t*)realptr;
  if (NFREELISTS == index)
    free(realptr);
  else if (index < NFREELISTS) {
    memory_t* free_block = (memory_t*)realptr;

    allocator_guard_t lock(spinlock_);
    free_block->next = free_list_[index];
    free_list_[index] = free_block;
  }
  else 
    abort();
}

allocator_t& 
allocator_t::singleton(void)
{
  static allocator_t _s_allocator;

  return _s_allocator;
}

}
