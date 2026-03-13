// Copyright (c) 2026 ASMlover. All rights reserved.
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
#include <iomanip>
#include <format>
#include "Debug.hh"
#include "Object.hh"

namespace ms {

namespace {

static sz_t simple_instruction(cstr_t name, sz_t offset) noexcept {
  std::cout << name << "\n";
  return offset + 1;
}

static sz_t byte_instruction(cstr_t name, const Chunk& chunk, sz_t offset) noexcept {
  u8_t slot = chunk.code_at(offset + 1);
  std::cout << std::format("{:<16s} {:4d}\n", name, slot);
  return offset + 2;
}

static sz_t constant_instruction(cstr_t name, const Chunk& chunk, sz_t offset) noexcept {
  u8_t index = chunk.code_at(offset + 1);
  std::cout << std::format("{:<16s} {:4d} '{}'\n",
      name, index, chunk.constant_at(index).stringify());
  return offset + 2;
}

static sz_t jump_instruction(cstr_t name, int sign, const Chunk& chunk, sz_t offset) noexcept {
  u8_t hi = chunk.code_at(offset + 1);
  u8_t lo = chunk.code_at(offset + 2);
  auto jump = static_cast<std::uint16_t>((hi << 8) | lo);
  sz_t target = static_cast<sz_t>(static_cast<int>(offset + 3) + sign * static_cast<int>(jump));
  std::cout << std::format("{:<16s} {:4d} -> {:04d}\n",
      name, offset, target);
  return offset + 3;
}

static sz_t invoke_instruction(cstr_t name, const Chunk& chunk, sz_t offset) noexcept {
  u8_t index = chunk.code_at(offset + 1);
  u8_t arg_count = chunk.code_at(offset + 2);
  std::cout << std::format("{:<16s} ({} args) {:4d} '{}'\n",
      name, arg_count, index, chunk.constant_at(index).stringify());
  return offset + 3;
}

static sz_t constant_long_instruction(cstr_t name, const Chunk& chunk, sz_t offset) noexcept {
  u32_t index = chunk.code_at(offset + 1);
  index |= static_cast<u32_t>(chunk.code_at(offset + 2)) << 8;
  index |= static_cast<u32_t>(chunk.code_at(offset + 3)) << 16;
  std::cout << std::format("{:<16s} {:4d} '{}'\n",
      name, index, chunk.constant_at(index).stringify());
  return offset + 4;
}

static sz_t closure_instruction(const Chunk& chunk, sz_t offset) noexcept {
  u8_t index = chunk.code_at(offset + 1);
  std::cout << std::format("{:<16s} {:4d} '{}'\n",
      "OP_CLOSURE", index, chunk.constant_at(index).stringify());
  offset += 2;

  // Print upvalue metadata
  Value constant = chunk.constant_at(index);
  if (constant.is_object() && constant.as_object()->type() == ObjectType::OBJ_FUNCTION) {
    ObjFunction* function = as_obj<ObjFunction>(constant.as_object());
    for (int j = 0; j < function->upvalue_count(); j++) {
      u8_t is_local = chunk.code_at(offset);
      u8_t upvalue_index = chunk.code_at(offset + 1);
      std::cout << std::format("{:04d}      |                     {} {}\n",
          offset, is_local ? "local" : "upvalue", upvalue_index);
      offset += 2;
    }
  }

  return offset;
}

} // anonymous namespace

void disassemble_chunk(const Chunk& chunk, strv_t name) noexcept {
  std::cout << std::format("== {} ==\n", name);

  for (sz_t offset = 0; offset < chunk.count();) {
    offset = disassemble_instruction(chunk, offset);
  }
}

sz_t disassemble_instruction(const Chunk& chunk, sz_t offset) noexcept {
  std::cout << std::format("{:04d} ", offset);

  if (offset > 0 && chunk.line_at(offset) == chunk.line_at(offset - 1)) {
    std::cout << "   | ";
  } else {
    std::cout << std::format("{:4d} ", chunk.line_at(offset));
  }

  u8_t byte = chunk.code_at(offset);
  auto op = static_cast<OpCode>(byte);

  switch (op) {
  // Simple instructions (no operands)
  case OpCode::OP_NIL:
  case OpCode::OP_TRUE:
  case OpCode::OP_FALSE:
  case OpCode::OP_POP:
  case OpCode::OP_EQUAL:
  case OpCode::OP_GREATER:
  case OpCode::OP_LESS:
  case OpCode::OP_ADD:
  case OpCode::OP_SUBTRACT:
  case OpCode::OP_MULTIPLY:
  case OpCode::OP_DIVIDE:
  case OpCode::OP_MODULO:
  case OpCode::OP_NOT:
  case OpCode::OP_NEGATE:
  case OpCode::OP_STR:
  case OpCode::OP_PRINT:
  case OpCode::OP_INDEX_GET:
  case OpCode::OP_INDEX_SET:
  case OpCode::OP_CLOSE_UPVALUE:
  case OpCode::OP_RETURN:
  case OpCode::OP_INHERIT:
  case OpCode::OP_IMPORT_FROM:
  case OpCode::OP_IMPORT_ALIAS:
    return simple_instruction(opcode_name(op), offset);

  // Byte instructions (1 byte operand = slot index)
  case OpCode::OP_GET_LOCAL:
  case OpCode::OP_SET_LOCAL:
  case OpCode::OP_GET_UPVALUE:
  case OpCode::OP_SET_UPVALUE:
  case OpCode::OP_BUILD_LIST:
  case OpCode::OP_BUILD_MAP:
  case OpCode::OP_CALL:
    return byte_instruction(opcode_name(op), chunk, offset);

  // Constant long instruction (3 byte operand = constant index)
  case OpCode::OP_CONSTANT_LONG:
    return constant_long_instruction(opcode_name(op), chunk, offset);

  // Constant instructions (1 byte operand = constant index)
  case OpCode::OP_CONSTANT:
  case OpCode::OP_GET_GLOBAL:
  case OpCode::OP_DEFINE_GLOBAL:
  case OpCode::OP_SET_GLOBAL:
  case OpCode::OP_GET_PROPERTY:
  case OpCode::OP_SET_PROPERTY:
  case OpCode::OP_GET_SUPER:
  case OpCode::OP_CLASS:
  case OpCode::OP_METHOD:
  case OpCode::OP_STATIC_METHOD:
  case OpCode::OP_IMPORT:
    return constant_instruction(opcode_name(op), chunk, offset);

  // Jump instructions (2 byte operand = jump offset, forward)
  case OpCode::OP_JUMP:
  case OpCode::OP_JUMP_IF_FALSE:
    return jump_instruction(opcode_name(op), 1, chunk, offset);

  // Loop instruction (2 byte operand = jump offset, backward)
  case OpCode::OP_LOOP:
    return jump_instruction(opcode_name(op), -1, chunk, offset);

  // Invoke instructions (constant index + arg count)
  case OpCode::OP_INVOKE:
  case OpCode::OP_SUPER_INVOKE:
    return invoke_instruction(opcode_name(op), chunk, offset);

  // Closure instruction
  case OpCode::OP_CLOSURE:
    return closure_instruction(chunk, offset);

  case OpCode::OP_FOR_ITER: {
    u8_t slot = chunk.code_at(offset + 1);
    u8_t hi = chunk.code_at(offset + 2);
    u8_t lo = chunk.code_at(offset + 3);
    auto jump = static_cast<std::uint16_t>((hi << 8) | lo);
    std::cout << std::format("{:<16s} {:4d} -> {:04d}\n",
        "OP_FOR_ITER", slot, offset + 4 + jump);
    return offset + 4;
  }

  default:
    std::cout << std::format("Unknown opcode {}\n", byte);
    return offset + 1;
  }
}

} // namespace ms
