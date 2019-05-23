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
#include <iostream>
#include "bytecc_chunk.hh"

namespace loxcc::bytecc {

sz_t Chunk::size_bytes(void) const {
  return sizeof(*this) +
    sizeof(u8_t) * codes_.capacity() +
    sizeof(int) * codes_.capacity() +
    sizeof(Value) * constants_.capacity();
}

void Chunk::dis(const str_t& name) {
  std::cout << "========= [" << name << "] =========" << std::endl;
  for (int i = 0; i < codes_count();)
    i = dis_ins(i);
}

int Chunk::dis_ins(int offset) {
  auto const_ins = [](Chunk* c, const char* s, int i) -> int {
    u8_t constant = c->get_code(i + 1);
    fprintf(stdout, "%-16s %4d ", s, constant);
    std::cout << "`" << c->get_constant(constant) << "`" << std::endl;
    return i + 2;
  };
  auto const_insN = [](Chunk* c, const str_t& s, int i, int n) -> int {
    u8_t constant = c->get_code(i + 1);
    fprintf(stdout, "%s_%-*d %4d ", s.c_str(), 15 - Xt::as_type<int>(s.size()), n, constant);
    std::cout << "`" << c->get_constant(constant) << "`" << std::endl;
    return i + 2;
  };
  auto simp_ins = [](const str_t& s, int i) -> int {
    std::cout << s << std::endl;
    return i + 1;
  };
  auto simp_insN = [](const str_t& s, int i, int n) -> int {
    std::cout << s << n << std::endl;
    return i + 1;
  };
  auto byte_ins = [](Chunk* c, const char* s, int i) -> int {
    fprintf(stdout, "%-16s %4d\n", s, c->get_code(i + 1));
    return i + 1;
  };
  auto jump_ins = [](Chunk* c, const char* s, int i, int sign) -> int {
    u16_t jump = Xt::as_type<u16_t>((c->get_code(i + 1) << 8) | c->get_code(i + 2));
    fprintf(stdout, "%-16s %4d -> %d\n", s, i, i + 3 + jump * sign);
    return i + 3;
  };

  fprintf(stdout, "%04d ", offset);
  if (offset > 0 && lines_[offset] == lines_[offset - 1])
    std::cout << "   | ";
  else
    fprintf(stdout, "%4d ", lines_[offset]);
  switch (Code c = Xt::as_type<Code>(codes_[offset])) {
  case Code::CONSTANT: return const_ins(this, "CONSTANT", offset);
  case Code::NIL: return simp_ins("NIL", offset);
  case Code::TRUE: return simp_ins("TRUE", offset);
  case Code::FALSE: return simp_ins("FALSE", offset);
  case Code::POP: return simp_ins("POP", offset);
  case Code::DEF_GLOBAL: return const_ins(this, "DEF_GLOBAL", offset);
  case Code::GET_GLOBAL: return const_ins(this, "GET_GLOBAL", offset);
  case Code::SET_GLOBAL: return const_ins(this, "SET_GLOBAL", offset);
  case Code::GET_LOCAL: return byte_ins(this, "GET_LOCAL", offset);
  case Code::SET_LOCAL: return byte_ins(this, "SET_LOCAL", offset);
  case Code::GET_UPVALUE: return byte_ins(this, "GET_UPVALUE", offset);
  case Code::SET_UPVALUE: return byte_ins(this, "SET_UPVALUE", offset);
  case Code::GET_ATTR: return const_ins(this, "GET_ATTR", offset);
  case Code::SET_ATTR: return const_ins(this, "SET_ATTR", offset);
  case Code::GET_SUPER: return const_ins(this, "GET_SUPER", offset);
  case Code::EQ: return simp_ins("EQ", offset);
  case Code::NE: return simp_ins("NE", offset);
  case Code::GT: return simp_ins("GT", offset);
  case Code::GE: return simp_ins("GE", offset);
  case Code::LT: return simp_ins("LT", offset);
  case Code::LE: return simp_ins("LE", offset);
  case Code::ADD: return simp_ins("ADD", offset);
  case Code::SUB: return simp_ins("SUB", offset);
  case Code::MUL: return simp_ins("MUL", offset);
  case Code::DIV: return simp_ins("DIV", offset);
  case Code::NOT: return simp_ins("NOT", offset);
  case Code::NEG: return simp_ins("NEG", offset);
  case Code::PRINT: return simp_ins("PRINT", offset);
  case Code::JUMP: return jump_ins(this, "JUMP", offset, 1);
  case Code::JUMP_IF_FALSE: return jump_ins(this, "JUMP_IF_FALSE", offset, 1);
  case Code::LOOP: return jump_ins(this, "LOOP", offset, -1);
  case Code::CALL_0:
  case Code::CALL_1:
  case Code::CALL_2:
  case Code::CALL_3:
  case Code::CALL_4:
  case Code::CALL_5:
  case Code::CALL_6:
  case Code::CALL_7:
  case Code::CALL_8: return simp_insN("CALL_", offset, c - Code::CALL_0);
  case Code::INVOKE_0:
  case Code::INVOKE_1:
  case Code::INVOKE_2:
  case Code::INVOKE_3:
  case Code::INVOKE_4:
  case Code::INVOKE_5:
  case Code::INVOKE_6:
  case Code::INVOKE_7:
  case Code::INVOKE_8: return const_insN(this, "INVOKE_", offset, c - Code::INVOKE_0);
  case Code::SUPER_0:
  case Code::SUPER_1:
  case Code::SUPER_2:
  case Code::SUPER_3:
  case Code::SUPER_4:
  case Code::SUPER_5:
  case Code::SUPER_6:
  case Code::SUPER_7:
  case Code::SUPER_8: return const_insN(this, "SUPER_", offset, c - Code::SUPER_0);
  case Code::CLOSURE: return const_ins(this, "CLOSURE", offset);
    // {
    //   offset = const_ins(this, "CLOSURE", offset);
    //   // TODO: print upvalues
    // }
  case Code::CLOSE_UPVALUE: return simp_ins("CLOSE_UPVALUE", offset);
  case Code::RETURN: return simp_ins("RETURN", offset);
  case Code::CLASS: return const_ins(this, "CLASS", offset);
  case Code::SUBCLASS: return simp_ins("SUBCLASS", offset);
  case Code::METHOD: return const_ins(this, "METHOD", offset);
  default: std::cerr << "UNKNOWN CODE: " << c << std::endl; break;
  }
  return offset + 1;
}

}
