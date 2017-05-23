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
#include <memory>

namespace gc {

using uchar_t = std::uint8_t;

struct Object;
class Worklist;

class HeapManager {
  enum {ROOT_COUNT = 1024};
  uchar_t* heaptr_{};
  uchar_t* allocptr_{};
  Object* roots_[ROOT_COUNT];
  std::size_t size_{};
  Object* freelist_{};
  std::unique_ptr<Worklist> worklist_;

  uchar_t* alloc(std::size_t& n);
  void dealloc(uchar_t* p);
  void mark_from_roots(void);
  void sweep(void);
public:
  static HeapManager& get_instance(void) {
    static HeapManager _ins;
    return _ins;
  }

  HeapManager(void);
  ~HeapManager(void);

  Object* new_int(int value);
  Object* new_pair(Object* first = nullptr, Object* second = nullptr);
  void collect(void);
};

}
