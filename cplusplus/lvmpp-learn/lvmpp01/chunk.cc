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
#include <cstdio>
#include <iostream>
#include "chunk.hh"

namespace lvm {

std::ostream& operator<<(std::ostream& out, OpCode code) {
  return out << EnumUtil<OpCode>::as_int(code);
}

void Chunk::write(OpCode byte) {
  codes_.push_back(byte);
}

void Chunk::disassemble(const std::string& name) {
  std::cout << "=========" << name << "=========" << std::endl;
  for (auto offset = 0; offset < static_cast<int>(codes_.size());)
    offset = disassemble_instruction(offset);
}

int Chunk::disassemble_instruction(int offset) {
  auto simple_instruction = [](const std::string& name, int offset) -> int {
    std::cout << name << std::endl;
    return offset + 1;
  };

  fprintf(stdout, "%04d ", offset);
  OpCode instruction = codes_[offset];
  switch (instruction) {
  case OpCode::OP_RETURN:
    return simple_instruction("OP_RETURN", offset);
  default:
    break;
  }

  std::cout << "unknown opcode: " << instruction << std::endl;
  return offset + 1;
}

}
