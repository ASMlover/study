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
#include "bytecc_value.hh"

namespace loxcc::bytecc {

enum class Code : u8_t {
  CONSTANT,

  NIL,
  TRUE,
  FALSE,
  POP,

  DEF_GLOBAL,
  GET_GLOBAL,
  SET_GLOBAL,
  GET_LOCAL,
  SET_LOCAL,
  GET_UPVALUE,
  SET_UPVALUE,
  GET_ATTR,
  SET_ATTR,

  GET_SUPER,

  EQ,
  NE,
  GT,
  GE,
  LT,
  LE,

  ADD,
  SUB,
  MUL,
  DIV,
  NOT,
  NEG,

  PRINT,

  JUMP,
  JUMP_IF_FALSE,
  LOOP,

  // calls and functions
  CALL_0,
  CALL_1,
  CALL_2,
  CALL_3,
  CALL_4,
  CALL_5,
  CALL_6,
  CALL_7,
  CALL_8,

  // methods and initializers
  INVOKE_0,
  INVOKE_1,
  INVOKE_2,
  INVOKE_3,
  INVOKE_4,
  INVOKE_5,
  INVOKE_6,
  INVOKE_7,
  INVOKE_8,

  // superclasses
  SUPER_0,
  SUPER_1,
  SUPER_2,
  SUPER_3,
  SUPER_4,
  SUPER_5,
  SUPER_6,
  SUPER_7,
  SUPER_8,

  CLOSURE,
  CLOSE_UPVALUE,
  RETURN,
  CLASS,
  SUBCLASS,
  METHOD,
};

inline u8_t operator+(Code a, Code b) {
  return Xt::as_type<u8_t>(a) + Xt::as_type<u8_t>(b);
}

inline u8_t operator-(Code a, Code b) {
  return Xt::as_type<u8_t>(a) - Xt::as_type<u8_t>(b);
}

inline Code operator+(Code c, u8_t n) {
  return Xt::as_type<Code>(Xt::as_type<u8_t>(c) + n);
}

inline Code operator-(Code c, u8_t n) {
  return Xt::as_type<Code>(Xt::as_type<u8_t>(c) - n);
}

class Chunk final : private UnCopyable {
  std::vector<u8_t> codes_;
  std::vector<int> lines_;
  std::vector<Value> constants_;
public:
  template <typename T> inline void write(T code, int line) {
    codes_.push_back(code);
    lines_.push_back(line);
  }

  inline u8_t add_constant(const Value& value) {
    constants_.push_back(value);
    return Xt::as_type<u8_t>(constants_.size() - 1);
  }

  inline int codes_count(void) const { return Xt::as_type<int>(codes_.size()); }
  inline const u8_t* codes(void) const { return codes_.data(); }
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline void set_code(int i, u8_t c) { codes_[i] = c; }
  inline int get_line(int i) const { return lines_[i]; }
  inline int constants_count(void) const { return Xt::as_type<int>(constants_.size()); }
  inline const Value* constants(void) const { return constants_.data(); }
  inline const Value& get_constant(int i) const { return constants_[i]; }

  inline void iter_constants(std::function<void (const Value&)>&& visitor) {
    for (auto& c : constants_)
      visitor(c);
  }

  sz_t size_bytes(void) const;
  void dis(const str_t& name);
  int dis_ins(int offset);
};

}
