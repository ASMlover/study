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

#include <string>
#include <vector>
#include "common.hh"
#include "value.hh"

namespace lvm {

enum class OpCode : int {
  OP_CONSTANT,
  OP_RETURN
};

std::ostream& operator<<(std::ostream& out, OpCode code);

class Chunk : public Copyable {
  std::vector<OpCode> codes_;
  std::vector<Value> constants_;
  std::vector<int> lines_;
public:
  OpCode add_constant(const Value& value);

  void write(OpCode code, int line);
  void write_constant(const Value& value, int line);

  void disassemble(const std::string& name);
  int disassemble_instruction(int offset);

  const OpCode* get_codes(void) const { return &codes_[0]; }
  OpCode get_code(int offset) const { return codes_[offset]; }
  Value get_constant(int constant) const { return constants_[constant]; }
  int get_line(int instruction) const { return lines_[instruction]; }
};

}
