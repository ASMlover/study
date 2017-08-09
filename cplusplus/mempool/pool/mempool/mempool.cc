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
#include <cstdlib>
#include "mempool.h"

struct MemBlock { MemBlock* next; };

MemPool::MemPool(void) {
}

MemPool::~MemPool(void) {
  for (auto& b : blocks_) {
    if (b.second.first != nullptr)
      std::free(b.second.first);
  }
  blocks_.clear();
}

MemBlock* MemPool::new_block(int index) {
  auto nbytes = index2bytes(index);

  if (freeblocks_[index] == nullptr) {
    auto* blockobj = std::malloc(POOL_SIZE);
    if (blockobj == nullptr)
      return nullptr;

    auto excess = (std::size_t)blockobj & SYSTEM_PAGE_SIZE_MASK;
    std::size_t alignment_bytes;
    if (excess != 0) {
      freeblocks_[index] =
        reinterpret_cast<MemBlock*>(
            (char*)blockobj + SYSTEM_PAGE_SIZE - excess);
      alignment_bytes =
        (POOL_SIZE / SYSTEM_PAGE_SIZE - 1) * SYSTEM_PAGE_SIZE - nbytes;
    }
    else {
      freeblocks_[index] = reinterpret_cast<MemBlock*>(blockobj);
      alignment_bytes = POOL_SIZE - nbytes;
    }
    MemBlock* block = freeblocks_[index];
    blocks_[block] = std::make_pair(blockobj, index);

    for (std::size_t i = 0u; i < alignment_bytes; i += nbytes)
      block = block->next = block + nbytes / sizeof(MemBlock);
    block->next = nullptr;
  }

  return freeblocks_[index];
}

void* MemPool::alloc(std::size_t nbytes) {
  void* p{};

  if (nbytes < SMALL_REQUEST_THRESHOLD) {
    auto index = bytes2index(nbytes);
    if (freeblocks_[index] == nullptr)
      new_block(index);

    p = freeblocks_[index];
    freeblocks_[index] = freeblocks_[index]->next;

    auto* aligned_block =
      (MemBlock*)((std::uintptr_t)p & ~SYSTEM_PAGE_SIZE_MASK);
    if (blocks_.find(aligned_block) == blocks_.end())
      blocks_[aligned_block] = std::make_pair(nullptr, index);
  }
  else {
    p = std::malloc(nbytes);
  }

  return p;
}

void MemPool::dealloc(void* p) {
  auto* block = (MemBlock*)((std::uintptr_t)p & ~SYSTEM_PAGE_SIZE_MASK);
  auto it = blocks_.find(block);
  if (it == blocks_.end()) {
    std::free(p);
  }
  else {
    auto index = it->second.second;
    auto* free_block = reinterpret_cast<MemBlock*>(p);
    free_block->next = freeblocks_[index];
    freeblocks_[index] = free_block;
  }
}
