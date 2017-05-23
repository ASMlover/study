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
#include "object.h"
#include "heap.h"

namespace gc {

constexpr std::size_t kHeapSize = 512 << 10;
constexpr std::size_t kMaxStack = 1024;

class Worklist {
  Object* worklist_[kMaxStack];
  std::size_t size_{};
public:
  bool is_full(void) const { return size_ >= kMaxStack; }
  bool is_empty(void) const { return size_ == 0; }
  void push(Object* obj) { worklist_[size_++] = obj; }
  Object* pop(void) { return worklist_[--size_]; }
};

HeapManager::HeapManager(void)
  : worklist_(new Worklist) {
  heaptr_ = new uchar_t[kHeapSize];
  if (heaptr_ == nullptr)
    std::abort();
  allocptr_ = heaptr_;
}

HeapManager::~HeapManager(void) {
  delete [] heaptr_;
  heaptr_ = allocptr_ = nullptr;
}

uchar_t* HeapManager::alloc(std::size_t& n) {
  if (allocptr_ + n <= heaptr_ + kHeapSize) {
    std::size_t leftsize = static_cast<std::size_t>(
        (heaptr_ + kHeapSize) - (allocptr_ + n));
    if (leftsize < kMinObjSize())
      n += leftsize;

    uchar_t* p = allocptr_;
    allocptr_ += n;
    return p;
  }

  if (freelist_ != nullptr) {
    void* p{};
    auto** block = &freelist_;
    while (*block != nullptr) {
      if ((*block)->size >= n) {
        p = *block;
        if ((*block)->size - n >= kMinObjSize()) {
          auto* nextobj = new (*block + n) Object;
          nextobj->size = (*block)->size - static_cast<std::uint16_t>(n);
          *block = nextobj;
        }
        else {
          *block = (*block)->next;
        }
        break;
      }
      else {
        block = &(*block)->next;
      }
    }

    if (p != nullptr)
      return static_cast<uchar_t*>(p);
  }

  return nullptr;
}

void HeapManager::collect(void) {
}

}
