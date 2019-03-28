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
#pragma once

#include <vector>
#include "common.hh"
#include "object.hh"

namespace nyx {

class VM : private UnCopyable {
  byte_t* heaptr_{};
  byte_t* fromspace_{};
  byte_t* tospace_{};
  byte_t* allocptr_{};
  std::vector<Value> stack_;

  static constexpr std::size_t kMaxHeap = (1 << 20) * 10;

  template <typename Source> inline Object* as_object(Source* x) const {
    return reinterpret_cast<Object*>(x);
  }

  template <typename Source> inline void* as_address(Source* x) const {
    return reinterpret_cast<void*>(x);
  }

  void initialize(void);
public:
  VM(void);
  ~VM(void);

  Value move_object(Value from_ref);
  void* allocate(std::size_t n);
  void collect(void);

  // void push_numeric(double value);
  // void push_pair(void);
  Value pop(void);

  void print_stack(void);
};

}