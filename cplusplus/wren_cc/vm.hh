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

namespace wrencc {

enum class ObjType {
  NUMERIC,
};

enum class ObjFlag {
  MARKED = 0x01,
};

class BaseObject : private UnCopyable {
  ObjType type_{};
  ObjFlag flag_{};
  union {
    double numeric;
  } as_{};
public:
  BaseObject(double d);

  str_t stringify(void) const;
};
using Value = BaseObject*;

inline Value make_value(double d) {
  return new BaseObject(d);
}

std::ostream& operator<<(std::ostream& out, Value val);

class Fiber : private UnCopyable {
  std::vector<Value> stack_;
public:
  inline void push(Value v) {
    stack_.push_back(v);
  }

  inline Value pop(void) {
    Value v = stack_.back();
    stack_.pop_back();
    return v;
  }
};

enum class Code : u8_t {
  CONSTANT, // load the constant at index [arg]

  END,
};

class Block : private UnCopyable {
  std::vector<u8_t> codes_;
  std::vector<Value> constants_;
public:
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline Value get_constant(int i) const { return constants_[i]; }

  template <typename T>
  inline void add_code(T c) { codes_.push_back(Xt::as_type<u8_t>(c)); }

  inline u8_t add_constant(Value v) {
    constants_.push_back(v);
    return Xt::as_type<u8_t>(constants_.size() - 1);
  }
};

class VM : private UnCopyable {
public:
  Value interpret(Fiber* fiber, Block* block);
};

}
