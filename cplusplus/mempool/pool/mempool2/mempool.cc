// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <cstdint>
#include <cstdlib>
#include "mempool.h"

struct BlockHeader {
  std::uintptr_t address;
  int size_index;
  int block_index;
};
struct MemBlock { MemBlock* next; };

MemPool::~MemPool(void) {
  for (auto* b : blocks_)
    std::free(b);
  blocks_.clear();
}

MemBlock* MemPool::new_block(int index) {
  auto* blockobj = reinterpret_cast<MemBlock*>(std::malloc(POOL_SIZE));
  if (blockobj == nullptr)
    return nullptr;
  blocks_.push_back(blockobj);

  std::size_t excess = static_cast<std::size_t>(
      (std::uintptr_t)blockobj & SYSTEM_PAGE_SIZE_MASK);
  std::size_t aligned_bytes;
  char* aligned_block;
  if (excess != 0) {
    aligned_block = (char*)blockobj + (SYSTEM_PAGE_SIZE - excess);
    aligned_bytes = PAGE_COUNT * SYSTEM_PAGE_SIZE;
  }
  else {
    aligned_block = (char*)blockobj;
    aligned_bytes = POOL_SIZE;
  }
  int block_header = sizeof(BlockHeader);
  freeblocks_[index] = reinterpret_cast<MemBlock*>(
      (char*)aligned_block + block_header);

  std::size_t nbytes = index2bytes(index);
  for (std::size_t i = 0; i < aligned_bytes; i += SYSTEM_PAGE_SIZE) {
    auto* h = (BlockHeader*)aligned_block;
    h->address = (std::uintptr_t)blockobj;
    h->size_index = index;
    h->block_index = i / SYSTEM_PAGE_SIZE;
    auto block_bytes = SYSTEM_PAGE_SIZE - block_header - nbytes;
    auto* block = reinterpret_cast<MemBlock*>(aligned_block + block_header);
    for (std::size_t s = 0; s < block_bytes; s += nbytes)
      block = block->next = block + nbytes / sizeof(MemBlock);
    block->next = nullptr;
    aligned_block += SYSTEM_PAGE_SIZE;
  }

  return freeblocks_[index];
}

void* MemPool::alloc(std::size_t nbytes) {
  void* p{};
  if ((nbytes - 1) < SMALL_REQUEST_THRESHOLD) {
    auto index = bytes2index(nbytes);
    if (freeblocks_[index] == nullptr)
      new_block(index);

    p = freeblocks_[index];
    if (p == nullptr)
      return nullptr;
    freeblocks_[index] = freeblocks_[index]->next;
  }
  else {
    p = std::malloc(nbytes);
  }

  return p;
}

void MemPool::dealloc(void* p) {
  auto* h = (BlockHeader*)((std::uintptr_t)p & ~SYSTEM_PAGE_SIZE_MASK);
  if (h->block_index < PAGE_COUNT
      && (std::uintptr_t)p - h->address < POOL_SIZE
      && h->address != 0) {
    auto index = h->size_index;
    auto* free_block = (MemBlock*)p;
    free_block->next = freeblocks_[index];
    freeblocks_[index] = free_block;
  }
  else {
    std::free(p);
  }
}
