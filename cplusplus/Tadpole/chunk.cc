// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <iostream>
#include "chunk.hh"

namespace tadpole {

inline sz_t dis_compound(Chunk* chunk,
  const char* msg, sz_t i, bool with_constant = false) noexcept {
  auto c = chunk->get_code(i + 1);
  std::fprintf(stdout, "%-16s %4d", msg, c);
  if (with_constant)
    std::cout << " `" << chunk->get_constant(c) << "`" << std::endl;
  return i + 2;
}

inline sz_t dis_simple(Chunk* chunk, const char* msg, sz_t i, int n = 0) noexcept {
  std::cout << msg;
  if (n > 0)
    std::cout << "_" << n;
  std::cout << std::endl;
  return i + 1;
}

void Chunk::dis(strv_t msg) {
  std::cout << "========= [" << msg << "] =========" << std::endl;
  for (sz_t offset = 0; offset < codes_.size();)
    offset = dis_code(offset);
}

sz_t Chunk::dis_code(sz_t offset) {
  std::fprintf(stdout, "%04d ", as_type<int>(offset));
  if (offset > 0 && lines_[offset] == lines_[offset - 1])
    std::fprintf(stdout, "   | ");
  else
    std::fprintf(stdout, "%4d ", lines_[offset]);

  switch (auto c = as_type<Code>(codes_[offset])) {
  case Code::CONSTANT: return dis_compound(this, "CONSTANT", offset, true);
  case Code::NIL: return dis_simple(this, "NIL", offset);
  case Code::FALSE: return dis_simple(this, "FALSE", offset);
  case Code::TRUE: return dis_simple(this, "TRUE", offset);
  case Code::POP: return dis_simple(this, "POP", offset);
  case Code::DEF_GLOBAL: return dis_compound(this, "DEF_GLOBAL", offset, true);
  case Code::GET_GLOBAL: return dis_compound(this, "GET_GLOBAL", offset, true);
  case Code::SET_GLOBAL: return dis_compound(this, "SET_GLOBAL", offset, true);
  case Code::GET_LOCAL: return dis_compound(this, "GET_LOCAL", offset);
  case Code::SET_LOCAL: return dis_compound(this, "SET_LOCAL", offset);
  case Code::GET_UPVALUE: return dis_compound(this, "GET_UPVALUE", offset);
  case Code::SET_UPVALUE: return dis_compound(this, "SET_UPVALUE", offset);
  case Code::ADD: return dis_simple(this, "ADD", offset);
  case Code::SUB: return dis_simple(this, "SUB", offset);
  case Code::MUL: return dis_simple(this, "MUL", offset);
  case Code::DIV: return dis_simple(this, "DIV", offset);
  case Code::CALL_0:
  case Code::CALL_1:
  case Code::CALL_2:
  case Code::CALL_3:
  case Code::CALL_4:
  case Code::CALL_5:
  case Code::CALL_6:
  case Code::CALL_7:
  case Code::CALL_8: return dis_simple(this, "CALL", offset, c - Code::CALL_0);
  case Code::CLOSURE: return dis_compound(this, "CLOSURE", offset);
  case Code::CLOSE_UPVALUE: return dis_simple(this, "CLOSE_UPVALUE", offset);
  case Code::RETURN: return dis_simple(this, "RETURN", offset);
  default: std::cerr << "<Invalid Code>" << std::endl; break;
  }
  return offset + 1;
}

}
