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
#include <functional>
#include <stack>
#include <vector>

namespace gc {

using uchar_t = std::uint8_t;

class Object;

class HeapManager {
  uchar_t* heaptr_{};
  uchar_t* allocptr_{};
  std::vector<Object*> roots_;
  std::stack<Object*> worklist_;
  std::size_t objcnt_{};

  uchar_t* alloc(std::size_t& n);
  Object* new_object(
      std::size_t n, const std::function<Object* (uchar_t*)>& fn);
  void mark(void);
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
  Object* pop_object(void);
  void collect(void);
};

}
