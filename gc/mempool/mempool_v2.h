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
#include <cstdlib>
#include <vector>

namespace v2 {

class MemoryPool {
  enum {
    ALIGN = 8, // 2 ^ 3
    ALIGN_SHIFT = 3,
    MAX_BYTES = 512,
    NFREELISTS = MAX_BYTES / ALIGN,
    MAX_NUMBER = 64,
  };

  struct Block {
    Block* next;
  };

  Block* freelist_[NFREELISTS];
  std::vector<Block*> chunklist_;

  MemoryPool(const MemoryPool&) = delete;
  MemoryPool& operator=(const MemoryPool&) = delete;

  static std::size_t index2bytes(std::size_t i) {
    return (i + 1) << ALIGN_SHIFT;
  }

  static std::size_t bytes2index(std::size_t bytes) {
    return (bytes - 1) >> ALIGN_SHIFT;
  }

  Block* alloc_chunk(std::size_t index) {
    std::size_t block_size = index2bytes(index);
    std::size_t chunk_size = block_size * MAX_NUMBER;

    if (!freelist_[index]) {
      freelist_[index] = (Block*)std::malloc(chunk_size);
      chunklist_.push_back(freelist_[index]);

      auto* block = freelist_[index];
      for (std::size_t i = 0; i < chunk_size - block_size; i += block_size)
        block = block->next = block + block_size / sizeof(Block);
      block->next = nullptr;
    }

    return freelist_[index];
  }

  MemoryPool(void) {
  }

  ~MemoryPool(void) {
    for (auto* block : chunklist_)
      std::free(block);
    chunklist_.clear();
  }
public:
  static MemoryPool& get_instance(void) {
    static MemoryPool ins;
    return ins;
  }

  void* alloc(std::size_t bytes) {
    if (bytes > MAX_BYTES) {
      return std::malloc(bytes);
    }
    else {
      std::size_t index = bytes2index(bytes);
      if (!freelist_[index])
        alloc_chunk(index);

      auto* p = freelist_[index];
      freelist_[index] = freelist_[index]->next;
      return p;
    }
  }

  void dealloc(void* p, std::size_t bytes) {
    if (bytes > MAX_BYTES) {
      free(p);
    }
    else {
      std::size_t index = bytes2index(bytes);
      auto* block = (Block*)p;
      block->next = freelist_[index];
      freelist_[index] = block;
    }
  }
};

}
