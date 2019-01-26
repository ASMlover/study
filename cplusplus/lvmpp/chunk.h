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

#include <cstdint>
#include <string>
#include <vector>
#include "common.h"
#include "value.h"

namespace lox {

enum OpCode {
  OP_CONSTANT,
  OP_NEGATIVE, // -
  OP_ADD, // binary +
  OP_SUBTRACT, // binary -
  OP_MULTIPLY, // binary *
  OP_DIVIDE, // binary /
  OP_RETURN
};

class Chunk : private Copyable {
  std::vector<std::uint8_t> codes_;
  std::vector<Value> constants_;
  std::vector<int> lines_;
public:
  void write(std::uint8_t byte, int line);
  int add_constant(const Value& value);

  void disassemble(const std::string& name);
  int disassemble_instruction(int offset);

  std::uint8_t get_code(int offset) const { return codes_[offset]; }
  Value get_constant(int constant) const { return constants_[constant]; }
};

}
