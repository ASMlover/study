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

namespace nyx {

enum OpCode {
  OP_CONSTANT,
  OP_NIL, // nil value
  OP_TRUE, // true value
  OP_FALSE, // false value
  OP_POP, // pop operation
  OP_GET_LOCAL, // get local variable
  OP_SET_LOCAL, // set local variable
  OP_DEF_GLOBAL, // define global variable
  OP_GET_GLOBAL, // get global variable
  OP_SET_GLOBAL, // set global variable
  OP_GET_UPVALUE, // get upvalue
  OP_SET_UPVALUE, // set upvalue
  OP_GET_FIELD, // get field
  OP_SET_FIELD, // set field
  OP_GET_SUPER, // get super
  OP_EQ, // ==
  OP_NE, // !=
  OP_GT, // >
  OP_GE, // >=
  OP_LT, // <
  OP_LE, // <=
  OP_ADD, // +
  OP_SUB, // -
  OP_MUL, // *
  OP_DIV, // /
  OP_NOT, // !
  OP_NEG, // - negative
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP, // loop flow
  OP_CALL_0,
  OP_CALL_1,
  OP_CALL_2,
  OP_CALL_3,
  OP_CALL_4,
  OP_CALL_5,
  OP_CALL_6,
  OP_CALL_7,
  OP_CALL_8,
  OP_INVOKE_0,
  OP_INVOKE_1,
  OP_INVOKE_2,
  OP_INVOKE_3,
  OP_INVOKE_4,
  OP_INVOKE_5,
  OP_INVOKE_6,
  OP_INVOKE_7,
  OP_INVOKE_8,
  OP_SUPER_0,
  OP_SUPER_1,
  OP_SUPER_2,
  OP_SUPER_3,
  OP_SUPER_4,
  OP_SUPER_5,
  OP_SUPER_6,
  OP_SUPER_7,
  OP_SUPER_8,
  OP_CLOSURE,
  OP_CLOSE_UPVALUE,
  OP_RETURN, // return
  OP_CLASS, // class
  OP_SUBCLASS, // subclass
  OP_METHOD,
};

class Chunk : private UnCopyable {
  std::vector<u8_t> codes_;
  std::vector<int> lines_;
  std::vector<Value> constants_;
public:
  int write(u8_t byte, int line);
  int add_constant(const Value& v);

  void disassemble(const str_t& name);
  int disassemble_instruction(int i);

  inline int codes_count(void) const { return static_cast<int>(codes_.size()); }
  inline u8_t* codes(void) { return codes_.data(); }
  inline const u8_t* codes(void) const { return codes_.data(); }
  inline void set_code(int i, u8_t byte) { codes_[i] = byte; }
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline int get_line(int i) const { return lines_[i]; }
  inline int constants_count(void) const { return static_cast<int>(constants_.size()); }
  inline Value* constants(void) { return constants_.data(); }
  inline const Value* constants(void) const { return constants_.data(); }
  inline Value& get_constant(int i) { return constants_[i]; }
  inline const Value& get_constant(int i) const { return constants_[i]; }

  inline void iter_constants(std::function<void (const Value&)>&& visit) {
    for (auto& c : constants_)
      visit(c);
  }
};

}
