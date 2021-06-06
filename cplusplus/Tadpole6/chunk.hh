// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
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

#include <vector>
#include "common.hh"
#include "value.hh"

namespace tadpole {

enum class Code : u8_t {
  CONSTANT,

  NIL,
  FALSE,
  TRUE,
  POP,

  DEF_GLOBAL,
  GET_GLOBAL,
  SET_GLOBAL,
  GET_LOCAL,
  SET_LOCAL,
  GET_UPVALUE,
  SET_UPVALUE,

  ADD,
  SUB,
  MUL,
  DIV,

  CALL_0,
  CALL_1,
  CALL_2,
  CALL_3,
  CALL_4,
  CALL_5,
  CALL_6,
  CALL_7,
  CALL_8,

  CLOSURE,

  CLOSE_UPVALUE,

  RETURN,
};

inline int operator-(Code a, Code b) noexcept {
  return as_type<int>(a) - as_type<int>(b);
}

template <typename T> inline Code operator+(Code a, T b) noexcept {
  return as_type<Code>(as_type<int>(a) + as_type<int>(b));
}

class Chunk final : private UnCopyable {
  std::vector<u8_t> codes_;
  std::vector<int> lines_;
  std::vector<Value> constants_;
public:
  template <typename T> inline u8_t write(T c, int lineno) noexcept {
    codes_.push_back(as_type<u8_t>(c));
    lines_.push_back(lineno);
    return as_type<u8_t>(codes_.size() - 1);
  }

  inline u8_t add_constant(Value value) noexcept {
    constants_.push_back(value);
    return as_type<u8_t>(constants_.size() - 1);
  }

  inline void write_constant(Value value, int lineno) noexcept {
    write(Code::CONSTANT, lineno);
    write(add_constant(value), lineno);
  }

  inline sz_t codes_count() const noexcept { return codes_.size(); }
  inline const u8_t* codes() const noexcept { return codes_.data(); }
  inline u8_t get_code(sz_t i) const noexcept { return codes_[i]; }
  template <typename T> inline void set_code(sz_t i, T c) noexcept { codes_[i] = as_type<u8_t>(c); }
  inline int get_line(sz_t i) const noexcept { return lines_[i]; }
  inline const Value& get_constant(sz_t i) const noexcept { return constants_[i]; }
  inline sz_t offset_with(const u8_t* ip) const noexcept { return as_type<sz_t>(ip - codes()); }

  template <typename Fn> inline void iter_constants(Fn&& fn) {
    for (auto& c : constants_)
      fn(c);
  }

  void dis(strv_t prompt) noexcept;
  sz_t dis_code(sz_t offset) noexcept;
};

}
