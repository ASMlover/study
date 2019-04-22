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
  static auto simple_instruction = [](const char* name) {
    std::cout << name << std::endl;
  };

  static auto simple_instructionN = [](const char* name, int n) {
    std::cout << name << n << std::endl;
  };

  static auto code_instruction = [](Chunk& c, int i, const char* name) -> int {
    u8_t slot = c.get_code(i++);
    fprintf(stdout, "%-16s %4d\n", name, slot);
    return i;
  };

  static auto const_instruction = [](Chunk& c, int i, const char* name) -> int {
    u8_t constant = c.get_code(i);
    fprintf(stdout, "%-16s %4d ", name, constant);
    std::cout << "`" << c.get_constant(constant) << "`" << std::endl;
    return i;
  };

  static auto const_instructionN = [](
      Chunk& c, int i, const char* name, int n) -> int {
    u8_t constant = c.get_code(i);
    fprintf(stdout, "%s%-*d %4d ",
        name, 15 - static_cast<int>(strlen(name)), n, constant);
    std::cout << "`" << c.get_constant(constant) << "`" << std::endl;
    return i;
  };

  static auto jump_instruction = [](
      Chunk& c, int i, const char* name, int sign = 1) -> int {
    u16_t offset = static_cast<u16_t>(c.get_code(i++) << 8);
    offset |= c.get_code(i++);
    fprintf(stdout, "%-16s %4d -> %d\n", name, offset, i + offset * sign);
    return i;
  };

  fprintf(stdout, "%04d ", i);
  if (i > 1 && lines_[i] == lines_[i - 1])
    std::cout << "   | ";
  else
    fprintf(stdout, "%4d ", lines_[i]);

  const u8_t* codes = codes_.data();
  switch (auto instruction = codes[i++]; instruction) {
  case OpCode::OP_CONSTANT:
    i = const_instruction(*this, i, "OP_CONSTANT"); break;
  case OpCode::OP_NIL: simple_instruction("OP_NIL"); break;
  case OpCode::OP_TRUE: simple_instruction("OP_TRUE"); break;
  case OpCode::OP_FALSE: simple_instruction("OP_FALSE"); break;
  case OpCode::OP_POP: simple_instruction("OP_POP"); break;
  case OpCode::OP_GET_LOCAL:
    i = code_instruction(*this, i, "OP_GET_LOCAL"); break;
  case OpCode::OP_SET_LOCAL:
    i = code_instruction(*this, i, "OP_SET_LOCAL"); break;
  case OpCode::OP_DEF_GLOBAL:
    i = const_instruction(*this, i, "OP_DEF_GLOBAL"); break;
  case OpCode::OP_GET_GLOBAL:
    i = const_instruction(*this, i, "OP_GET_GLOBAL"); break;
  case OpCode::OP_SET_GLOBAL:
    i = const_instruction(*this, i, "OP_SET_GLOBAL"); break;
  case OpCode::OP_GET_UPVALUE:
    i = code_instruction(*this, i, "OP_GET_UPVALUE"); break;
  case OpCode::OP_SET_UPVALUE:
    i = code_instruction(*this, i, "OP_SET_UPVALUE"); break;
  case OpCode::OP_GET_FIELD:
    i = const_instruction(*this, i, "OP_GET_FIELD"); break;
  case OpCode::OP_SET_FIELD:
    i = const_instruction(*this, i, "OP_SET_FIELD"); break;
  case OpCode::OP_GET_SUPER:
    i = const_instruction(*this, i, "OP_GET_SUPER"); break;
  case OpCode::OP_EQ: simple_instruction("OP_EQ"); break;
  case OpCode::OP_NE: simple_instruction("OP_NE"); break;
  case OpCode::OP_GT: simple_instruction("OP_GT"); break;
  case OpCode::OP_GE: simple_instruction("OP_GE"); break;
  case OpCode::OP_LT: simple_instruction("OP_LT"); break;
  case OpCode::OP_LE: simple_instruction("OP_LE"); break;
  case OpCode::OP_ADD: simple_instruction("OP_ADD"); break;
  case OpCode::OP_SUB: simple_instruction("OP_SUB"); break;
  case OpCode::OP_MUL: simple_instruction("OP_MUL"); break;
  case OpCode::OP_DIV: simple_instruction("OP_DIV"); break;
  case OpCode::OP_NOT: simple_instruction("OP_NOT"); break;
  case OpCode::OP_NEG: simple_instruction("OP_NEG"); break;
  case OpCode::OP_JUMP:
    i = jump_instruction(*this, i, "OP_JUMP"); break;
  case OpCode::OP_JUMP_IF_FALSE:
    i = jump_instruction(*this, i, "OP_JUMP_IF_FALSE"); break;
  case OpCode::OP_LOOP:
    i = jump_instruction(*this, i, "OP_LOOP", -1); break;
  case OpCode::OP_CALL_0:
  case OpCode::OP_CALL_1:
  case OpCode::OP_CALL_2:
  case OpCode::OP_CALL_3:
  case OpCode::OP_CALL_4:
  case OpCode::OP_CALL_5:
  case OpCode::OP_CALL_6:
  case OpCode::OP_CALL_7:
  case OpCode::OP_CALL_8:
    simple_instructionN("OP_CALL_", instruction - OpCode::OP_CALL_0); break;
  case OpCode::OP_INVOKE_0:
  case OpCode::OP_INVOKE_1:
  case OpCode::OP_INVOKE_2:
  case OpCode::OP_INVOKE_3:
  case OpCode::OP_INVOKE_4:
  case OpCode::OP_INVOKE_5:
  case OpCode::OP_INVOKE_6:
  case OpCode::OP_INVOKE_7:
  case OpCode::OP_INVOKE_8:
    i = const_instructionN(*this, i, "OP_INVOKE_", instruction - OpCode::OP_INVOKE_0);
    break;
  case OpCode::OP_SUPER_0:
  case OpCode::OP_SUPER_1:
  case OpCode::OP_SUPER_2:
  case OpCode::OP_SUPER_3:
  case OpCode::OP_SUPER_4:
  case OpCode::OP_SUPER_5:
  case OpCode::OP_SUPER_6:
  case OpCode::OP_SUPER_7:
  case OpCode::OP_SUPER_8:
    i = const_instructionN(*this, i, "OP_SUPER_", instruction - OpCode::OP_SUPER_0);
    break;
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
    } break;
  case OpCode::OP_CLOSE_UPVALUE: simple_instruction("OP_CLOSE_UPVALUE"); break;
  case OpCode::OP_RETURN: simple_instruction("OP_RETURN"); break;
  case OpCode::OP_CLASS:
    i = const_instruction(*this, i, "OP_CLASS"); break;
  case OpCode::OP_SUBCLASS:
    i = const_instruction(*this, i, "OP_SUBCLASS"); break;
  case OpCode::OP_METHOD:
    i = const_instruction(*this, i, "OP_METHOD"); break;
  }
  return i;
}

}
