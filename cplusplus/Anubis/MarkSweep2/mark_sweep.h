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

#include <functional>
#include <vector>
#include <stack>
#include "object.h"

namespace gc {

class MarkSweep : private Chaos::UnCopyable {
  static constexpr std::size_t kChunkCount = 64;
  static constexpr std::size_t kHeapSize = 512 << 10;
  static constexpr std::size_t kAlignment = sizeof(void*);
  static constexpr std::size_t kFreelistCount = 512 / kAlignment;

  struct MemoryChunk { int index; MemoryHeader* chunk; };

  MemoryHeader* freelist_[kFreelistCount]{};
  byte_t* heaptr_{};
  byte_t* allocptr_{};
  std::vector<MemoryChunk> chunklist_;
  std::vector<BaseObject*> roots_;
  std::stack<BaseObject*> worklist_;
  std::size_t obj_count_{};

  static constexpr int as_index(std::size_t n) {
    return static_cast<int>((n + 1) / kAlignment);
  }

  static constexpr std::size_t as_bytes(int index) {
    return (index + 1) * kAlignment;
  }

  MarkSweep(void);
  ~MarkSweep(void);

  void* alloc(std::size_t n);
  BaseObject* new_object(
      std::size_t n, const std::function<BaseObject* (void*)>& fn);
  void mark(void);
  void mark_from_roots(void);
  void sweep(void);
public:
  static MarkSweep& get_instance(void);

  void collect(void);
  BaseObject* create_int(int value = 0);
  BaseObject* create_pair(
      BaseObject* first = nullptr, BaseObject* second = nullptr);
  BaseObject* release_object(void);
};

}
