// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___           ____    ____
// /\__  _\           /\ \                /\_ \         /\  _`\ /\  _`\
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __\ \ \L\_\ \ \/\_\
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\ \ \L_L\ \ \/_/_
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/\ \ \/, \ \ \L\ \
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\\ \____/\ \____/
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/ \/___/  \/___/
//                             \ \_\
//                              \/_/
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

#include <exception>
#include <iostream>
#include <vector>
#include <tadpole/common/common.hh>
#include "memory_header.hh"

namespace tadpole::gc {

class MemoryChunk final : public MemoryHeader {
  sz_t size_{};
  MemoryChunk* next_{};
public:
  MemoryChunk(sz_t n) noexcept : MemoryHeader(MemoryTag::MEMORY), size_(n) {}

  inline sz_t size() const noexcept { return size_; }
  inline sz_t set_size(sz_t n) noexcept { return size_ = n, size_; }
  inline sz_t inc_size(sz_t n) noexcept { return size_ += n, size_; }
  inline sz_t dec_size(sz_t n) noexcept { return size_ -= n, size_; }
  inline MemoryChunk* next() const noexcept { return next_; }
  inline void set_next(MemoryChunk* next) noexcept { next_ = next; }
};

class BaseObject;

class MarkSweep final : public Singleton<MarkSweep> {
  static constexpr sz_t kHeapSize = 512 << 10;

  byte_t* heapptr_{};
  byte_t* freeptr_{};
  MemoryChunk* freelist_{};

  std::vector<BaseObject*> roots_;
  std::vector<BaseObject*> worklist_;

  void mark_from_roots();
  void mark();
  void sweep();
public:
  MarkSweep() noexcept;
  ~MarkSweep() noexcept;

  void* alloc(sz_t n) noexcept;
  void collect();

  template <typename Obj, typename... Args> inline Obj* create_object(Args&&... args) {
    sz_t n = sizeof(Obj);
    void* ref = alloc(n);
    if (ref == nullptr) {
      collect();
      ref = alloc(n);

      if (ref == nullptr)
        throw std::exception("out of memory");
    }

    Obj* o = new (ref) Obj(std::forward<Args>(args)...);
    roots_.push_back(o);
    return o;
  }
};

template <typename T> inline T* as_ptr(void* p) noexcept { return reinterpret_cast<T*>(p); }
inline MemoryTag as_tag(void* p) noexcept { return as_ptr<MemoryHeader>(p)->tag(); }

}
