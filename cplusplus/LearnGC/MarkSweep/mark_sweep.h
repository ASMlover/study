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
#include <Chaos/Types.h>

namespace gc {

class BaseObject;

class MarkSweep : private Chaos::UnCopyable {
  byte_t* heaptr_{};
  byte_t* allocptr_{};
  std::vector<BaseObject*> roots_;
  std::stack<BaseObject*> worklist_;
  std::size_t obj_count_{};
  static constexpr std::size_t kHeapSize = 512 << 10;

  MarkSweep(void);
  ~MarkSweep(void);

  byte_t* alloc(std::size_t& n);
  BaseObject* create_object(
      std::size_t n, std::function<BaseObject* (byte_t*)>&& fn);
  void mark(void);
  void mark_from_roots(void);
  void sweep(void);
public:
  static MarkSweep& get_instance(void);

  void collect(void);
  BaseObject* put_in(int value);
  BaseObject* put_in(BaseObject* first = nullptr, BaseObject* second = nullptr);
  BaseObject* fetch_out(void);
};

}
