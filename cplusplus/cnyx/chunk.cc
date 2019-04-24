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
#include "value.hh"
#include "object.hh"
#include "chunk.hh"

namespace nyx {

int Chunk::write(u8_t byte, int line) {
  codes_.push_back(byte);
  lines_.push_back(line);

  return static_cast<int>(codes_.size() - 1);
}

int Chunk::add_constant(const Value& v) {
  constants_.push_back(v);
  return static_cast<int>(constants_.size() - 1);
}

void Chunk::disassemble(const str_t& name) {
  std::cout << "=== " << name << " ===" << std::endl;
  for (int i = 0; i < static_cast<int>(codes_.size());)
    i = disassemble_instruction(i);
}

int Chunk::disassemble_instruction(int i) {
  static auto simple_instruction = [](int i, const char* name) -> int {
    std::cout << name << std::endl;
    return i;
  };

  static auto simple_instructionN = [](int i, const char* name, int n) -> int {
    std::cout << name << n << std::endl;
    return i;
  };

  static auto code_instruction = [](Chunk& c, int i, const char* name) -> int {
    u8_t slot = c.get_code(i);
    fprintf(stdout, "%-16s %4d\n", name, slot);
    return i + 1;
  };

  static auto const_instruction = [](Chunk& c, int i, const char* name) -> int {
    u8_t constant = c.get_code(i);
    fprintf(stdout, "%-16s %4d ", name, constant);
    std::cout << "`" << c.get_constant(constant) << "`" << std::endl;
    return i + 1;
  };

  static auto const_instructionN = [](
      Chunk& c, int i, const char* name, int n) -> int {
    u8_t constant = c.get_code(i);
    fprintf(stdout, "%s%-*d %4d ",
        name, 15 - static_cast<int>(strlen(name)), n, constant);
    std::cout << "`" << c.get_constant(constant) << "`" << std::endl;
    return i + 1;
  };

  static auto jump_instruction = [](
      Chunk& c, int i, const char* name, int sign = 1) -> int {
    u16_t offset = static_cast<u16_t>(c.get_code(i) << 8);
    offset |= c.get_code(i + 1);
    fprintf(stdout, "%-16s %4d -> %d\n", name, offset, i + offset * sign);
    return i + 2;
  };

  fprintf(stdout, "%04d ", i);
  if (i > 1 && lines_[i] == lines_[i - 1])
    std::cout << "   | ";
  else
    fprintf(stdout, "%4d ", lines_[i]);

  const u8_t* codes = codes_.data();
  switch (auto instruction = codes[i++]; instruction) {
  case OpCode::OP_CONSTANT: return const_instruction(*this, i, "OP_CONSTANT");
  case OpCode::OP_NIL: return simple_instruction(i, "OP_NIL");
  case OpCode::OP_TRUE: return simple_instruction(i, "OP_TRUE");
  case OpCode::OP_FALSE: return simple_instruction(i, "OP_FALSE");
  case OpCode::OP_POP: return simple_instruction(i, "OP_POP");
  case OpCode::OP_GET_LOCAL: return code_instruction(*this, i, "OP_GET_LOCAL");
  case OpCode::OP_SET_LOCAL: return code_instruction(*this, i, "OP_SET_LOCAL");
  case OpCode::OP_DEF_GLOBAL: return const_instruction(*this, i, "OP_DEF_GLOBAL");
  case OpCode::OP_GET_GLOBAL: return const_instruction(*this, i, "OP_GET_GLOBAL");
  case OpCode::OP_SET_GLOBAL: return const_instruction(*this, i, "OP_SET_GLOBAL");
  case OpCode::OP_GET_UPVALUE: return code_instruction(*this, i, "OP_GET_UPVALUE");
  case OpCode::OP_SET_UPVALUE: return code_instruction(*this, i, "OP_SET_UPVALUE");
  case OpCode::OP_GET_FIELD: return const_instruction(*this, i, "OP_GET_FIELD");
  case OpCode::OP_SET_FIELD: return const_instruction(*this, i, "OP_SET_FIELD");
  case OpCode::OP_GET_SUPER: return const_instruction(*this, i, "OP_GET_SUPER");
  case OpCode::OP_EQ: return simple_instruction(i, "OP_EQ");
  case OpCode::OP_NE: return simple_instruction(i, "OP_NE");
  case OpCode::OP_GT: return simple_instruction(i, "OP_GT");
  case OpCode::OP_GE: return simple_instruction(i, "OP_GE");
  case OpCode::OP_LT: return simple_instruction(i, "OP_LT");
  case OpCode::OP_LE: return simple_instruction(i, "OP_LE");
  case OpCode::OP_ADD: return simple_instruction(i, "OP_ADD");
  case OpCode::OP_SUB: return simple_instruction(i, "OP_SUB");
  case OpCode::OP_MUL: return simple_instruction(i, "OP_MUL");
  case OpCode::OP_DIV: return simple_instruction(i, "OP_DIV");
  case OpCode::OP_NOT: return simple_instruction(i, "OP_NOT");
  case OpCode::OP_NEG: return simple_instruction(i, "OP_NEG");
  case OpCode::OP_JUMP: return jump_instruction(*this, i, "OP_JUMP");
  case OpCode::OP_JUMP_IF_FALSE: return jump_instruction(*this, i, "OP_JUMP_IF_FALSE");
  case OpCode::OP_LOOP: return jump_instruction(*this, i, "OP_LOOP", -1);
  case OpCode::OP_CALL_0:
  case OpCode::OP_CALL_1:
  case OpCode::OP_CALL_2:
  case OpCode::OP_CALL_3:
  case OpCode::OP_CALL_4:
  case OpCode::OP_CALL_5:
  case OpCode::OP_CALL_6:
  case OpCode::OP_CALL_7:
  case OpCode::OP_CALL_8:
    return simple_instructionN(i, "OP_CALL_", instruction - OpCode::OP_CALL_0);
  case OpCode::OP_INVOKE_0:
  case OpCode::OP_INVOKE_1:
  case OpCode::OP_INVOKE_2:
  case OpCode::OP_INVOKE_3:
  case OpCode::OP_INVOKE_4:
  case OpCode::OP_INVOKE_5:
  case OpCode::OP_INVOKE_6:
  case OpCode::OP_INVOKE_7:
  case OpCode::OP_INVOKE_8:
    return const_instructionN(*this, i, "OP_INVOKE_", instruction - OpCode::OP_INVOKE_0);
  case OpCode::OP_SUPER_0:
  case OpCode::OP_SUPER_1:
  case OpCode::OP_SUPER_2:
  case OpCode::OP_SUPER_3:
  case OpCode::OP_SUPER_4:
  case OpCode::OP_SUPER_5:
  case OpCode::OP_SUPER_6:
  case OpCode::OP_SUPER_7:
  case OpCode::OP_SUPER_8:
    return const_instructionN(*this, i, "OP_SUPER_", instruction - OpCode::OP_SUPER_0);
  case OpCode::OP_CLOSURE:
    {
      u8_t constant = codes[i++];
      fprintf(stdout, "%-16s %4d ", "OP_CLOSURE", constant);
      Value constant_val = constants_[constant];
      std::cout << "`" << constant_val << "`" << std::endl;

      FunctionObject* closed_fn = constant_val.as_function();
      for (int j = 0; j < closed_fn->upvalues_count(); ++j) {
        int is_local = codes[i++];
        int index = codes[i++];
        fprintf(stdout, "%04d   |                     %s %d\n",
            i - 2, is_local ? "local" : "upvalue", index);
      }
      return i;
    }
  case OpCode::OP_CLOSE_UPVALUE: return simple_instruction(i, "OP_CLOSE_UPVALUE");
  case OpCode::OP_RETURN: return simple_instruction(i, "OP_RETURN");
  case OpCode::OP_CLASS: return const_instruction(*this, i, "OP_CLASS");
  case OpCode::OP_SUBCLASS: return const_instruction(*this, i, "OP_SUBCLASS");
  case OpCode::OP_METHOD: return const_instruction(*this, i, "OP_METHOD");
  default: return i;
  }
  return i;
}

}
