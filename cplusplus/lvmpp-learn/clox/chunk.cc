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
#include <iostream>
#include "chunk.hh"
#include "object.hh"

namespace clox {

inline sz_t dis_compound(
    Chunk* chunk, const char* prompt, sz_t i, bool with_constant = false) noexcept {
  auto c = chunk->get_code(i + 1);
  std::fprintf(stdout, "%-16s %4d", prompt, c);
  if (with_constant)
    std::cout << " `" << chunk->get_constant(c) << "`";
  std::cout << std::endl;

  return i + 2;
}

inline sz_t dis_simple(Chunk* chunk, const char* prompt, sz_t i, int n = 0) noexcept {
  std::cout << prompt;
  if (n > 0)
    std::cout << "_" << n;
  std::cout << std::endl;

  return i + 1;
}

inline sz_t dis_jump(Chunk* chunk, const char* prompt, int i, int sign) noexcept {
  u16_t jump = (as_type<u16_t>(chunk->get_code(i + 1)) << 8) | chunk->get_code(i + 2);
  std::fprintf(stdout, "%-16s %4d -> %d\n", prompt, i, i + 3 + sign * jump);
  return i + 3;
}

void Chunk::dis(strv_t prompt) noexcept {
  std::cout << "========= [" << prompt << "] =========" << std::endl;
  for (sz_t offset = 0; offset < codes_count();)
    offset = dis_code(offset);
}

sz_t Chunk::dis_code(sz_t offset) noexcept {
  std::fprintf(stdout, "%04d ", as_type<int>(offset));
  if (offset > 0 && lines_[offset] == lines_[offset - 1])
    std::fprintf(stdout, "   | ");
  else
    std::fprintf(stdout, "%4d ", lines_[offset]);

#define COMPOUND(x)     return dis_compound(this, #x, offset)
#define COMPOUND2(x, b) return dis_compound(this, #x, offset, (b))
#define SIMPLE(x)       return dis_simple(this, #x, offset)
#define SIMPLE2(x, n)   return dis_simple(this, #x, offset, (n))
#define JUMP(x, s)      return dis_jump(this, #x, as_type<int>(offset), (s))

  switch (auto c = as_type<OpCode>(codes_[offset])) {
  case OpCode::OP_CONSTANT: COMPOUND2(OP_CONSTANT, true);
  case OpCode::OP_NIL: SIMPLE(OP_NIL);
  case OpCode::OP_TRUE: SIMPLE(OP_TRUE);
  case OpCode::OP_FALSE: SIMPLE(OP_FALSE);
  case OpCode::OP_POP: SIMPLE(OP_POP);
  case OpCode::OP_GET_LOCAL: COMPOUND(OP_GET_LOCAL);
  case OpCode::OP_SET_LOCAL: COMPOUND(OP_SET_LOCAL);
  case OpCode::OP_GET_GLOBAL: COMPOUND2(OP_GET_GLOBAL, true);
  case OpCode::OP_DEFINE_GLOBAL: COMPOUND2(OP_DEFINE_GLOBAL, true);
  case OpCode::OP_SET_GLOBAL: COMPOUND2(OP_SET_GLOBAL, true);
  case OpCode::OP_GET_UPVALUE: SIMPLE(OP_GET_UPVALUE);
  case OpCode::OP_SET_UPVALUE: SIMPLE(OP_SET_UPVALUE);
  case OpCode::OP_EQUAL: SIMPLE(OP_EQUAL);
  case OpCode::OP_GREATER: SIMPLE(OP_GREATER);
  case OpCode::OP_LESS: SIMPLE(OP_LESS);
  case OpCode::OP_ADD: SIMPLE(OP_ADD);
  case OpCode::OP_SUBTRACT: SIMPLE(OP_SUBTRACT);
  case OpCode::OP_MULTIPLY: SIMPLE(OP_MULTIPLY);
  case OpCode::OP_DIVIDE: SIMPLE(OP_DIVIDE);
  case OpCode::OP_NOT: SIMPLE(OP_NOT);
  case OpCode::OP_NEGATE: SIMPLE(OP_NEGATE);
  case OpCode::OP_PRINT: SIMPLE(OP_PRINT);
  case OpCode::OP_JUMP: JUMP(OP_JUMP, 1);
  case OpCode::OP_JUMP_IF_FALSE: JUMP(OP_JUMP_IF_FALSE, 1);
  case OpCode::OP_LOOP: JUMP(OP_LOOP, -1);
  case OpCode::OP_CALL: SIMPLE(OP_CALL);
  case OpCode::OP_CLOSURE:
    {
      offset++;
      auto c = get_code(offset++);
      std::fprintf(stdout, "%-16s %4d", "OP_CLOSURE", c);
      std::cout << " `" << get_constant(c) << "`" << std::endl;

      ObjFunction* function = get_constant(c).as_function();
      for (int j = 0; j < function->upvalue_count(); ++j) {
        int is_local = get_code(offset++);
        int index = get_code(offset++);
        std::fprintf(stdout, "%04d      |                     %s %d\n",
            as_type<int>(offset - 2), is_local ? "local" : "upvalue", index);
      }
      return offset;
    } break;
  case OpCode::OP_CLOSE_UPVALUE: SIMPLE(OP_CLOSE_UPVALUE);
  case OpCode::OP_RETURN: SIMPLE(OP_RETURN);
  default: std::cerr << "<Invalid `OpCode`>" << std::endl;
  }

#undef JUMP
#undef SIMPLE2
#undef SIMPLE
#undef COMPOUND2
#undef COMPOUND

  return offset + 1;
}

}
