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

#include <functional>
#include <vector>
#include "common.hh"

namespace wrencc {

enum class ObjType {
  NUMERIC,
};

enum class ObjFlag {
  MARKED = 0x01,
};

class BaseObject;
using Value = BaseObject*;

class BaseObject : private UnCopyable {
  ObjType type_{};
  ObjFlag flag_{};
  union {
    double numeric;
  } as_{};
public:
  BaseObject(double d);

  inline double as_numeric(void) const { return as_.numeric; }
  str_t stringify(void) const;

  static Value make_numeric(double d);
};

inline Value make_value(double d) {
  return new BaseObject(d);
}

std::ostream& operator<<(std::ostream& out, Value val);

enum class Code : u8_t {
  CONSTANT, // load the constant at index [arg]
  POP, // pop and discard the top of stack
  LOAD_LOCAL, // push the value in local slot [arg]
  STORE_LOCAL, // pop and store the value in local slot [arg]
  CALL, // invoke the method with symbol [arg]

  END,
};

class Block : private UnCopyable {
  std::vector<u8_t> codes_;
  std::vector<Value> constants_;
  int num_locals_{};
public:
  inline int num_locals(void) const { return num_locals_; }
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline Value get_constant(int i) const { return constants_[i]; }
  inline void set_num_locals(int n) { num_locals_ = n; }

  template <typename T>
  inline void add_code(T c) { codes_.push_back(Xt::as_type<u8_t>(c)); }

  inline u8_t add_constant(Value v) {
    constants_.push_back(v);
    return Xt::as_type<u8_t>(constants_.size() - 1);
  }
};

using PrimitiveFn = std::function<Value (Value receiver)>;

class VM;

class Class : private UnCopyable {
  std::vector<PrimitiveFn> methods_;

  static constexpr sz_t kMaxMethods = 256;
public:
  Class(void) : methods_(kMaxMethods) {
    for (auto i = 0; i < kMaxMethods; ++i)
      methods_[i] = nullptr;
  }

  inline PrimitiveFn get_method(int i) const { return methods_[i]; }
  void append_method(VM& vm, const str_t& name, PrimitiveFn&& fn);
};

class VM : private UnCopyable {
  std::vector<str_t> symbols_;
  Class num_class_;

  Value interpret(Block* block);
public:
  VM(void);

  int ensure_symbol(const str_t& name);
  int add_symbol(const str_t& name);
  int get_symbol(const str_t& name) const;
  void clear_symbol(void);

  void interpret(const str_t& source_bytes);
};

}
