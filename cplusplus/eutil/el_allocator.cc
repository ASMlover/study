// Copyright (c) 2014 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include "el_util.h"
#include "el_allocator.h"



namespace el {

struct Memory {
  Memory* next;
};



Memory* Allocator::AllocChunk(size_t index) {
  size_t alloc_size = (index + 1) * ALIGN;
  size_t chunk_size = alloc_size * MAX_NUMBER;

  if (nullptr == free_list_[index]) {
    free_list_[index] = static_cast<Memory*>(malloc(chunk_size));
    EL_ASSERT(nullptr != free_list_[index]);

    Memory* node = free_list_[index];
    for (size_t i = 0; i < chunk_size - alloc_size; i += alloc_size)
      node = node->next = node + (index + 1) * ALIGN / sizeof(*node);
    node->next = nullptr;
  }

  return free_list_[index];
}

void Allocator::InsertChunk(void* chunk) {
  if (chunk_count_ == chunk_storage_) {
    size_t new_chunk_storage = chunk_storage_ + NFREELISTS;
    void** new_chunk_list = 
      static_cast<void**>(malloc(sizeof(void*) * new_chunk_storage));
    EL_ASSERT(nullptr != new_chunk_list);

    memmove(new_chunk_list, chunk_list_, sizeof(void*) * chunk_storage_);
    free(chunk_list_);
    chunk_list_ = new_chunk_list;
    chunk_storage_ = new_chunk_storage;
  }

  chunk_list_[chunk_count_++] = chunk;
}


Allocator::Allocator(void) {
  memset(free_list_, 0, sizeof(free_list_));
  chunk_list_ = static_cast<void**>(malloc(sizeof(void*) * NFREELISTS));
  EL_ASSERT(nullptr != chunk_list_);
  chunk_count_ = 0;
  chunk_storage_ = NFREELISTS;
}

Allocator::~Allocator(void) {
  for (size_t i = 0; i < chunk_count_; ++i)
    free(chunk_list_[i]);
  free(chunk_list_);
}

void* Allocator::Alloc(size_t bytes) {
  // Allocator must has been initialized 
  // and the bytes must > 0
  EL_ASSERT(bytes > 0);

  void* ret;
  if (bytes > MAX_BYTES) {
    ret = malloc(bytes);
  }
  else {
    size_t index = ChunkIndex(bytes);

    LockerGuard<SpinLock> guard(locker_);
    if (nullptr == free_list_[index])
      AllocChunk(index);

    ret = free_list_[index];
    free_list_[index] = free_list_[index]->next;
  }

  return ret;
}

void Allocator::Dealloc(void* ptr, size_t bytes) {
  EL_ASSERT(nullptr != ptr && bytes > 0);

  if (bytes > MAX_BYTES) {
    free(ptr);
  }
  else {
    size_t index = ChunkIndex(bytes);
    Memory* free_block = static_cast<Memory*>(ptr);

    LockerGuard<SpinLock> guard(locker_);
    free_block->next = free_list_[index];
    free_list_[index] = free_block;
  }
}

}
