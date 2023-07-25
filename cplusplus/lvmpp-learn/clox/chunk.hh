// Copyright (c) 2023 ASMlover. All rights reserved.
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

#include <algorithm>
#include <vector>
#include "common.hh"
#include "value.hh"

namespace clox {

enum class OpCode : u8_t {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_RETURN,
};

inline int operator-(OpCode a, OpCode b) noexcept {
  return as_type<int>(a) - as_type<int>(b);
}

template <typename N> inline OpCode operator+(OpCode a, N b) noexcept {
  return as_type<OpCode>(as_type<int>(a) + as_type<int>(b));
}

inline std::ostream& operator<<(std::ostream& out, OpCode code) noexcept {
  return out << as_type<int>(code);
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

  inline u8_t add_constant(const Value& value) noexcept {
    constants_.push_back(value);
    return as_type<u8_t>(constants_.size() -  1);
  }

  inline void write_constant(Value value, int lineno) noexcept {
    write(OpCode::OP_CONSTANT, lineno);
    write(add_constant(value), lineno);
  }

  inline sz_t codes_count() const noexcept { return codes_.size(); }
  inline const u8_t* codes() const noexcept { return codes_.data(); }
  inline u8_t get_code(sz_t i) const noexcept { return codes_[i]; }
  template <typename T> inline void set_code(sz_t i, T c) noexcept { codes_[i] = as_type<u8_t>(c); }
  inline int get_line(sz_t i) const noexcept { return lines_[i]; }
  inline const Value& get_constant(sz_t i) const noexcept { return constants_[i]; }
  inline sz_t offset_from(const u8_t* ip) const noexcept { return as_type<sz_t>(ip - codes()); }

  template <typename Fn> inline void iter_constants(Fn fn) noexcept {
    std::for_each(constants_.begin(), constants_.end(), fn);
  }

  void dis(strv_t prompt) noexcept;
  sz_t dis_code(sz_t offset) noexcept;
};

}
