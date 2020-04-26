// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <algorithm>
#include <cstdlib>
#include <Core/Memory/InfantAllocator.hh>

namespace _mevo {

struct MemoryBlock {
  MemoryBlock* next;
};

MemoryBlock* InfantAllocator::alloc_chunk(sz_t index) {
  sz_t alloc_sz = (index + 1) * kAlign;
  sz_t chunk_sz = std::min(kMaxBytes / alloc_sz * alloc_sz, alloc_sz * kNumber);

  if (freelist_[index] == nullptr) {
    freelist_[index] = reinterpret_cast<MemoryBlock*>(std::malloc(chunk_sz));
    chunks_.push_back(freelist_[index]);

    MemoryBlock* iter = freelist_[index];
    for (sz_t i = 0; i < chunk_sz - alloc_sz; i += alloc_sz)
      iter = iter->next = iter + (index + 1) * kAlign / sizeof(*iter);
    iter->next = nullptr;
  }
  return freelist_[index];
}

InfantAllocator::InfantAllocator(void) noexcept {
}

InfantAllocator::~InfantAllocator(void) noexcept {
  for (auto* mb : chunks_)
    std::free(mb);
  chunks_.clear();
}

void* InfantAllocator::alloc(sz_t n) {
  void* p;
  if (n > kMaxBytes) {
    p = std::malloc(n);
  }
  else {
    sz_t index = as_index(n);

    p = freelist_[index];
    if (freelist_[index] == nullptr)
      p = alloc_chunk(index);
    freelist_[index] = freelist_[index]->next;
  }

  return p;
}

void InfantAllocator::dealloc(void* p, sz_t n) {
  if (n > kMaxBytes) {
    std::free(p);
  }
  else {
    sz_t index = as_index(n);
    MemoryBlock* mb = reinterpret_cast<MemoryBlock*>(p);
    mb->next = freelist_[index];
    freelist_[index] = mb;
  }
}

}
