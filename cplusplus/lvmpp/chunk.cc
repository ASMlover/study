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
#include "chunk.h"

namespace lox {

static int simple_instruction(const std::string& name, int offset) {
  std::cout << name << std::endl;
  return offset + 1;
}

static int constant_instruction(
    const std::string& name, const Chunk& chunk, int offset) {
  std::uint8_t constant = chunk.get_code(offset + 1);
  fprintf(stdout, "%-16s %4d `", name.c_str(), constant);
  std::cout << chunk.get_constant(constant);
  fprintf(stdout, "`\n");

  return offset + 2;
}

static int byte_instruction(
    const std::string& name, const Chunk& chunk, int offset) {
  std::uint8_t slot = chunk.get_code(offset + 1);
  fprintf(stdout, "%-16s %4d\n", name.c_str(), slot);
  return offset + 2;
}

static int jump_instruction(
    const std::string& name, const Chunk& chunk, int offset, int sign) {
  std::uint16_t jump =
    static_cast<std::uint16_t>(chunk.get_code(offset + 1) << 8);
  jump |= chunk.get_code(offset + 2);
  fprintf(stdout, "%-16s %4d -> %d\n",
      name.c_str(), offset, offset + 3 + sign * jump);
  return offset + 3;
}

void Chunk::write(std::uint8_t byte, int line) {
  codes_.push_back(byte);
  lines_.push_back(line);
}

int Chunk::add_constant(const Value& value) {
  constants_.push_back(value);
  return static_cast<int>(constants_.size() - 1);
}

void Chunk::disassemble(const std::string& name) {
  std::cout << "========= " << name << " =========" << std::endl;
  for (auto offset = 0; offset < static_cast<int>(codes_.size());)
    offset = disassemble_instruction(offset);
}

int Chunk::disassemble_instruction(int offset) {
  fprintf(stdout, "%04d ", offset);
  if (offset > 0 && lines_[offset] == lines_[offset - 1])
    fprintf(stdout, "   | ");
  else
    fprintf(stdout, "%4d ", lines_[offset]);

  std::uint8_t instruction = codes_[offset];
  switch (instruction) {
  case OpCode::OP_CONSTANT:
    return constant_instruction("OP_CONSTANT", *this, offset);
  case OpCode::OP_NEGATIVE:
    return simple_instruction("OP_NEGATIVE", offset);
  case OpCode::OP_NOT:
    return simple_instruction("OP_NOT", offset);
  case OpCode::OP_NIL:
    return simple_instruction("OP_NIL", offset);
  case OpCode::OP_TRUE:
    return simple_instruction("OP_TRUE", offset);
  case OpCode::OP_FALSE:
    return simple_instruction("OP_FALSE", offset);
  case OpCode::OP_POP:
    return simple_instruction("OP_POP", offset);
  case OpCode::OP_GET_LOCAL:
    return byte_instruction("OP_GET_LOCAL", *this, offset);
  case OpCode::OP_SET_LOCAL:
    return byte_instruction("OP_SET_LOCAL", *this, offset);
  case OpCode::OP_GET_GLOBAL:
    return constant_instruction("OP_GET_GLOBAL", *this, offset);
  case OpCode::OP_SET_GLOBAL:
    return constant_instruction("OP_SET_GLOBAL", *this, offset);
  case OpCode::OP_DEFINE_GLOBAL:
    return constant_instruction("OP_DEFINE_GLOBAL", *this, offset);
  case OpCode::OP_EQUAL:
    return simple_instruction("OP_EQUAL", offset);
  case OpCode::OP_GREATER:
    return simple_instruction("OP_GREATER", offset);
  case OpCode::OP_LESS:
    return simple_instruction("OP_LESS", offset);
  case OpCode::OP_ADD:
    return simple_instruction("OP_ADD", offset);
  case OpCode::OP_SUBTRACT:
    return simple_instruction("OP_SUBTRACT", offset);
  case OpCode::OP_MULTIPLY:
    return simple_instruction("OP_MULTIPLY", offset);
  case OpCode::OP_DIVIDE:
    return simple_instruction("OP_DIVIDE", offset);
  case OpCode::OP_PRINT:
    return simple_instruction("OP_PRINT", offset);
  case OpCode::OP_JUMP_IF_FALSE:
    return jump_instruction("OP_JUMP_IF_FALSE", *this, offset, 1);
  case OpCode::OP_RETURN:
    return simple_instruction("OP_RETURN", offset);
  default:
    break;
  }

  std::cout << "unknown opcode: " << instruction << std::endl;
  return offset + 1;
}

}
