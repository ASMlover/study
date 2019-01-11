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
#include <iomanip>
#include <iostream>
#include "chunk.h"

std::size_t Chunk::simple_instruction(const char* name, std::size_t offset) {
  std::fprintf(stdout, "\t%s\n", name);
  return offset + 1;
}

std::size_t Chunk::constant_instruction(const char* name, std::size_t offset) {
  auto constant = chunk_[offset + 1];
  std::fprintf(stdout, "\t%-16s %4u %lf\n",
      name, constant, constants_[constant]);
  return offset + 2;
}

std::size_t Chunk::disassemble_instruction(std::size_t offset) {
  std::fprintf(stdout, "%04lu", offset);
  if (offset > 0 && lines_[offset] == lines_[offset - 1])
    std::fprintf(stdout, "%9c ", '|');
  else
    std::fprintf(stdout, "%8d| ", lines_[offset]);

  std::uint8_t inst = chunk_[offset];
  switch (inst) {
  case OP_CONSTANT:
    return constant_instruction("OP_CONSTANT", offset);
  case OP_RETURN:
    return simple_instruction("OP_RETURN", offset);
  default:
    std::cout << "unknown opcode: " << inst << std::endl;
    return offset;
  }
}

void Chunk::write_chunk(std::uint8_t byte, int line) {
  chunk_.push_back(byte);
  lines_.push_back(line);
}

std::size_t Chunk::add_constant(Value value) {
  constants_.push_back(value);
  return constants_.size() - 1;
}

void Chunk::free_chunk(void) {
  chunk_.clear();
  lines_.clear();
  constants_.clear();
}

void Chunk::disassemble(const char* name) {
  std::cout << "====== " << name << " ======" << std::endl;

  auto n = chunk_.size();
  for (auto i = 0u; i < n;)
    i = disassemble_instruction(i);
}
