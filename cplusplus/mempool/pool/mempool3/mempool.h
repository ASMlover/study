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
#pragma once

#include <cstdint>
#include <cstdlib>
#include <vector>
#include "mutex.h"

template <typename MutexType = FakeMutex>
class MemPool {
  enum {
    ALIGNMENT = 8,
    ALIGNMENT_SHIFT = 3,
    SMALL_REQUEST_THRESHOLD = 512,
    NB_SMALL_SIZE_CLASSES = SMALL_REQUEST_THRESHOLD / ALIGNMENT,
    SYSTEM_PAGE_SIZE = 4 << 10,
    SYSTEM_PAGE_SIZE_MASK = SYSTEM_PAGE_SIZE - 1,
    POOL_SIZE = 256 << 10,
    PAGE_COUNT = POOL_SIZE / SYSTEM_PAGE_SIZE,
  };

  struct BlockHeader { std::uintptr_t address; int bi; int si; };
  struct MemBlock { MemBlock* next; };

  mutable MutexType mutex_;
  MemBlock* freeblocks_[NB_SMALL_SIZE_CLASSES]{};
  std::vector<void*> blocks_;

  class MemGuard {
    MemPool* mp_{};
    MemGuard(const MemGuard&) = delete;
    MemGuard& operator=(const MemGuard&) = delete;
  public:
    MemGuard(MemPool* mp) : mp_(mp) { mp_->mutex_.lock(); }
    ~MemGuard(void) { mp_->mutex_.unlock(); }
  };
  MemPool(const MemPool&) = delete;
  MemPool& operator=(const MemPool&) = delete;

  inline std::size_t index2bytes(int index) {
    return static_cast<std::size_t>((index + 1) << ALIGNMENT_SHIFT);
  }

  inline int bytes2index(std::size_t nbytes) {
    return static_cast<int>((nbytes - 1) >> ALIGNMENT_SHIFT);
  }

  MemPool(void) {}
  ~MemPool(void) {
    for (auto* address : blocks_)
      std::free(address);
    blocks_.clear();
  }

  MemBlock* new_block(int index) {
    auto* address = (char*)std::malloc(POOL_SIZE);
    if (address == nullptr)
      return nullptr;
    blocks_.push_back(address);

    std::size_t excess = (std::uintptr_t)address & SYSTEM_PAGE_SIZE_MASK;
    char* aligned_address = address;
    std::size_t aligned_count = PAGE_COUNT;
    if (excess != 0) {
      aligned_address = address + SYSTEM_PAGE_SIZE - excess;
      aligned_count = PAGE_COUNT - 1;
    }
    auto headsz = sizeof(BlockHeader);
    freeblocks_[index] = (MemBlock*)(aligned_address + headsz);

    auto nbytes = index2bytes(index);
    for (auto i = 0; i < (int)aligned_count; ++i) {
      auto* head = (BlockHeader*)aligned_address;
      head->address = (std::uintptr_t)aligned_address;
      head->bi = i;
      head->si = index;

      auto* block = (MemBlock*)(aligned_address + headsz);
      auto block_bytes = SYSTEM_PAGE_SIZE - headsz - nbytes;
      for (std::size_t sz = 0; sz < block_bytes; sz += nbytes)
        block = block->next = block + nbytes / sizeof(MemBlock);
      block->next = nullptr;

      aligned_address += SYSTEM_PAGE_SIZE;
    }

    return freeblocks_[index];
  }
public:
  static MemPool& get_instance(void) {
    static MemPool ins;
    return ins;
  }

  void* alloc(std::size_t nbytes) {
    void* p{};

    if ((nbytes - 1) < SMALL_REQUEST_THRESHOLD) {
      auto index = bytes2index(nbytes);

      MemGuard g(this);
      if (freeblocks_[index] == nullptr)
        new_block(index);

      p = freeblocks_[index];
      if (p != nullptr)
        freeblocks_[index] = freeblocks_[index]->next;
    }
    else {
      p = std::malloc(nbytes);
    }

    return p;
  }

  void dealloc(void* p) {
    if (p == nullptr)
      return;

    auto* head = (BlockHeader*)((std::uintptr_t)p & ~SYSTEM_PAGE_SIZE_MASK);
    if (head->bi < PAGE_COUNT && head->address != 0
        && (std::uintptr_t)p - head->address < POOL_SIZE) {
      auto index = head->si;
      auto* reclaim_block = (MemBlock*)p;

      MemGuard g(this);
      reclaim_block->next = freeblocks_[index];
      freeblocks_[index] = reclaim_block;
    }
  }
};
