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

#include <cstddef>
#include <cstdint>

namespace v1 {

struct Block {
  std::uint16_t bytes;
  std::uint16_t nfree;
  std::uint16_t first{1};
  std::uint16_t dummy;
  Block* next{};
  char data[1];

  static void* operator new(std::size_t,
      std::size_t unit_size, std::uint16_t unit_count) {
    return ::operator new(sizeof(Block) + unit_size * unit_count);
  }

  static void operator delete(void* p, std::size_t) {
    ::operator delete(p);
  }

  Block(std::size_t unit_size, std::uint16_t unit_count = 1)
    : bytes(unit_size * unit_count)
    , nfree(unit_count - 1) {
    auto* p = data;
    for (std::uint16_t i = 1; i < unit_count; ++i) {
      *(std::uint16_t*)p = i;
      p += unit_size;
    }
  }

  ~Block(void) {
  }
};

#define MEMPOOL_ALIGNMENT 4

class MemoryPool {
  Block* block_{};
  std::uint16_t unit_size_{};
  std::uint16_t init_count_{};
  std::uint16_t grow_count_{};

  MemoryPool(const MemoryPool&) = delete;
  MemoryPool& operator=(const MemoryPool&) = delete;

  void release_block(Block* block) {
    while (block) {
      auto* temp = block->next;
      block = block->next;
      delete temp;
    }
  }
public:
  MemoryPool(std::uint16_t unit_size,
      std::uint16_t init_count, std::uint16_t grow_count)
    : init_count_(init_count)
    , grow_count_(grow_count) {
    if (unit_size <= 2) {
      unit_size_ = 2;
    }
    else if (unit_size > 2 && unit_size <= 4) {
      unit_size_ = 4;
    }
    else {
      if (unit_size % MEMPOOL_ALIGNMENT == 0)
        unit_size_ = unit_size;
      else
        unit_size_ = (unit_size / MEMPOOL_ALIGNMENT + 1) * MEMPOOL_ALIGNMENT;
    }
  }

  ~MemoryPool(void) {
    if (block_)
      release_block(block_);
  }

  void* alloc(void) {
    if (!block_) {
      block_ = new(unit_size_, init_count_) Block(unit_size_, init_count_);
      return block_->data;
    }

    auto* block = block_;
    while (block && !block->nfree)
      block = block->next;

    if (block) {
      char* p = block->data + (block->first * unit_size_);
      block->first = *(std::uint16_t*)p;
      --block->nfree;
      return (void*)p;
    }
    else {
      if (!grow_count_)
        return nullptr;

      block = new(unit_size_, grow_count_) Block(unit_size_, grow_count_);
      if (!block)
        return nullptr;

      block->next = block_;
      block_ = block;

      return (void*)block->data;
    }
  }

  void dealloc(void* p) {
    auto* block = block_;
    Block* pre_block{};
    while ((std::uintptr_t)p < (std::uintptr_t)block->data ||
        (std::uintptr_t)p > (std::uintptr_t)(block->data + block->bytes)) {
      pre_block = block;
      block = block->next;
      if (!block)
        return;
    }

    if (block) {
      ++block->nfree;
      *(std::uint16_t*)p = block->first;
      block->first = (std::uint16_t)(((std::uintptr_t)p - (std::uintptr_t)block->data) / unit_size_);

      if (block->nfree * unit_size_ == block->bytes) {
        if (!block->next) {
          delete block;
          if (pre_block)
            pre_block->next = nullptr;
          else
            block_ = nullptr;
        }
      }
    }
  }
};

}
