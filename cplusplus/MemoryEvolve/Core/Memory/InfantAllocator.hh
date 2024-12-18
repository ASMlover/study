// Copyright (c) 2020 ASMlover. All rights reserved.
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

#include <vector>
#include <Core/MEvolve.hh>

namespace _mevo {

struct MemoryBlock;

class InfantAllocator final : private UnCopyable {
  static constexpr sz_t kAlign = 8;
  static constexpr sz_t kMaxBytes = 1 << 12;
  static constexpr sz_t kNumber = 64;
  static constexpr sz_t kFreelist = kMaxBytes / kAlign;

  MemoryBlock* freelist_[kFreelist]{};
  std::vector<MemoryBlock*> chunks_;

  inline sz_t as_index(sz_t bytes) const {
    return (bytes + kAlign - 1) / kAlign - 1;
  }

  InfantAllocator(void) noexcept;
  ~InfantAllocator(void) noexcept;

  MemoryBlock* alloc_chunk(sz_t index);
public:
  static InfantAllocator& instance(void) {
    static InfantAllocator ins;
    return ins;
  }

  void* alloc(sz_t n);
  void dealloc(void* p, sz_t n);
};

}
