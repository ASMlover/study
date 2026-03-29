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

// Helper: format an RK operand as either R(n) or K(n)
static str_t rk_str(u8_t rk, const Chunk& chunk) noexcept {
  if (is_rk_const(rk)) {
    u8_t ki = rk_to_const(rk);
    return std::format("K({}) '{}'", ki, chunk.constant_at(ki).stringify());
  }
  return std::format("R({})", rk);
}

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

  Instruction instr = chunk.code_at(offset);
  OpCode op = decode_op(instr);
  u8_t A = decode_A(instr);
  u8_t B = decode_B(instr);
  u8_t C = decode_C(instr);
  u16_t Bx = decode_Bx(instr);
  int sBx = decode_sBx(instr);

  switch (op) {
  // --- iABx: loading constants, globals, closures, classes ---
  case OpCode::OP_LOADK:
    std::cout << std::format("{:<16s} R({}) := K({}) '{}'\n",
        opcode_name(op), A, Bx, chunk.constant_at(Bx).stringify());
    return offset + 1;

  case OpCode::OP_GETGLOBAL:
  case OpCode::OP_SETGLOBAL:
  case OpCode::OP_DEFGLOBAL:
    std::cout << std::format("{:<16s} R({})  K({}) '{}'\n",
        opcode_name(op), A, Bx, chunk.constant_at(Bx).stringify());
    return offset + 1;

  case OpCode::OP_CLASS:
    std::cout << std::format("{:<16s} R({}) := class '{}'\n",
        opcode_name(op), A, chunk.constant_at(Bx).stringify());
    return offset + 1;

  // --- iA: simple register instructions ---
  case OpCode::OP_LOADTRUE:
  case OpCode::OP_LOADFALSE:
  case OpCode::OP_PRINT:
  case OpCode::OP_THROW:
  case OpCode::OP_IMPORT:
  case OpCode::OP_DEFER:
    std::cout << std::format("{:<16s} R({})\n", opcode_name(op), A);
    return offset + 1;

  case OpCode::OP_LOADNIL:
    std::cout << std::format("{:<16s} R({})..R({})\n", opcode_name(op), A, A + B);
    return offset + 1;

  // --- iABC: register movement ---
  case OpCode::OP_MOVE:
    std::cout << std::format("{:<16s} R({}) := R({})\n", opcode_name(op), A, B);
    return offset + 1;

  // --- iABC: upvalues ---
  case OpCode::OP_GETUPVAL:
    std::cout << std::format("{:<16s} R({}) := upval[{}]\n", opcode_name(op), A, B);
    return offset + 1;
  case OpCode::OP_SETUPVAL:
    std::cout << std::format("{:<16s} upval[{}] := R({})\n", opcode_name(op), B, A);
    return offset + 1;

  // --- iABC: properties with IC slot in C (GETPROP/INVOKE) or B (SETPROP) ---
  // C/B == 0xFF: fallback with EXTRAARG carrying ic_slot as u16_t
  case OpCode::OP_GETPROP: {
    if (C != 0xFF) {
      std::cout << std::format("{:<16s} R({}) := R({}). [ic:{}]\n",
          opcode_name(op), A, B, C);
      return offset + 1;
    }
    Instruction extra = chunk.code_at(offset + 1);
    u16_t ic = decode_Bx(extra);
    std::cout << std::format("{:<16s} R({}) := R({}). [ic:{}] (fallback)\n",
        opcode_name(op), A, B, ic);
    return offset + 2;
  }
  case OpCode::OP_SETPROP: {
    if (B != 0xFF) {
      std::cout << std::format("{:<16s} R({}).[ic:{}] := R({})\n",
          opcode_name(op), A, B, C);
      return offset + 1;
    }
    Instruction extra = chunk.code_at(offset + 1);
    u16_t ic = decode_Bx(extra);
    std::cout << std::format("{:<16s} R({}).[ic:{}] := R({}) (fallback)\n",
        opcode_name(op), A, ic, C);
    return offset + 2;
  }

  case OpCode::OP_GETSUPER:
    std::cout << std::format("{:<16s} R({}) := super(R({})).K({}) '{}'\n",
        opcode_name(op), A, B, C, chunk.constant_at(C).stringify());
    return offset + 1;

  // --- iABC: arithmetic/comparison with RK encoding ---
  case OpCode::OP_ADD: case OpCode::OP_SUB:
  case OpCode::OP_MUL: case OpCode::OP_DIV: case OpCode::OP_MOD:
  case OpCode::OP_EQ:  case OpCode::OP_LT:  case OpCode::OP_LE:
  case OpCode::OP_BAND: case OpCode::OP_BOR: case OpCode::OP_BXOR:
  case OpCode::OP_SHL: case OpCode::OP_SHR:
  // Quickened variants
  case OpCode::OP_ADD_II: case OpCode::OP_ADD_FF: case OpCode::OP_ADD_SS:
  case OpCode::OP_SUB_II: case OpCode::OP_SUB_FF:
  case OpCode::OP_MUL_II: case OpCode::OP_MUL_FF:
  case OpCode::OP_DIV_FF:
  case OpCode::OP_LT_II: case OpCode::OP_LT_FF:
  case OpCode::OP_EQ_II:
    std::cout << std::format("{:<16s} R({}) := {} op {}\n",
        opcode_name(op), A, rk_str(B, chunk), rk_str(C, chunk));
    return offset + 1;

  // --- iABC: unary ---
  case OpCode::OP_NEG:
  case OpCode::OP_NOT:
  case OpCode::OP_STR:
  case OpCode::OP_BNOT:
    std::cout << std::format("{:<16s} R({}) := op R({})\n", opcode_name(op), A, B);
    return offset + 1;

  // --- iAsBx: control flow ---
  case OpCode::OP_JMP:
    std::cout << std::format("{:<16s} PC += {} -> {:04d}\n",
        opcode_name(op), sBx, static_cast<int>(offset + 1) + sBx);
    return offset + 1;

  case OpCode::OP_TEST:
    std::cout << std::format("{:<16s} if bool(R({})) != {} then PC++\n",
        opcode_name(op), A, C);
    return offset + 1;

  case OpCode::OP_TESTSET:
    std::cout << std::format("{:<16s} if bool(R({})) == {} then R({}):=R({}) else PC++\n",
        opcode_name(op), B, C, A, B);
    return offset + 1;

  // --- iABC: function calls ---
  case OpCode::OP_CALL:
    std::cout << std::format("{:<16s} R({}) := R({})(R({})..R({})) ; {} args\n",
        opcode_name(op), A, A, A + 1, A + B - 1, B - 1);
    return offset + 1;

  case OpCode::OP_INVOKE: {
    if (C != 0xFF) {
      std::cout << std::format("{:<16s} R({}).({} args) [ic:{}]\n",
          opcode_name(op), A, B, C);
      return offset + 1;
    }
    Instruction extra = chunk.code_at(offset + 1);
    u16_t ic = decode_Bx(extra);
    std::cout << std::format("{:<16s} R({}).({} args) [ic:{}] (fallback)\n",
        opcode_name(op), A, B, ic);
    return offset + 2;
  }

  case OpCode::OP_SUPERINV:
    std::cout << std::format("{:<16s} super.K({}) '{}' base=R({}) ({} args)\n",
        opcode_name(op), C, chunk.constant_at(C).stringify(), A, B);
    return offset + 1;

  case OpCode::OP_RETURN:
    if (B >= 2) {
      std::cout << std::format("{:<16s} R({})\n", opcode_name(op), A);
    } else {
      std::cout << std::format("{:<16s} (nil)\n", opcode_name(op));
    }
    return offset + 1;

  case OpCode::OP_CLOSURE: {
    std::cout << std::format("{:<16s} R({}) := closure K({}) '{}'\n",
        opcode_name(op), A, Bx, chunk.constant_at(Bx).stringify());
    sz_t next = offset + 1;

    Value constant = chunk.constant_at(Bx);
    if (constant.is_object() && constant.as_object()->type() == ObjectType::OBJ_FUNCTION) {
      ObjFunction* function = as_obj<ObjFunction>(constant.as_object());
      for (int j = 0; j < function->upvalue_count(); j++) {
        Instruction uv = chunk.code_at(next);
        u8_t is_local = decode_A(uv);
        u8_t uv_index = static_cast<u8_t>(decode_Bx(uv));
        std::cout << std::format("{:04d}      |                     {} {}\n",
            next, is_local ? "local" : "upvalue", uv_index);
        next++;
      }
    }
    return next;
  }

  case OpCode::OP_CLOSE:
    std::cout << std::format("{:<16s} close >= R({})\n", opcode_name(op), A);
    return offset + 1;

  // --- iABC: OOP ---
  case OpCode::OP_INHERIT:
    std::cout << std::format("{:<16s} R({}).inherit(R({}))\n", opcode_name(op), A, B);
    return offset + 1;

  case OpCode::OP_METHOD:
  case OpCode::OP_STATICMETH:
  case OpCode::OP_GETTER:
  case OpCode::OP_SETTER:
  case OpCode::OP_ABSTMETH:
    std::cout << std::format("{:<16s} class(R({})).K({}) '{}' := R({})\n",
        opcode_name(op), A, B, chunk.constant_at(B).stringify(), C);
    return offset + 1;

  // --- iABC: collections ---
  case OpCode::OP_NEWLIST:
    std::cout << std::format("{:<16s} R({}) := [R({})..R({})] ({} elems)\n",
        opcode_name(op), A, A + 1, A + B, B);
    return offset + 1;
  case OpCode::OP_NEWMAP:
    std::cout << std::format("{:<16s} R({}) := {{...}} ({} pairs)\n",
        opcode_name(op), A, B);
    return offset + 1;
  case OpCode::OP_NEWTUPLE:
    std::cout << std::format("{:<16s} R({}) := (R({})..R({})) ({} elems)\n",
        opcode_name(op), A, A + 1, A + B, B);
    return offset + 1;

  case OpCode::OP_GETIDX:
    std::cout << std::format("{:<16s} R({}) := R({})[R({})]\n", opcode_name(op), A, B, C);
    return offset + 1;
  case OpCode::OP_SETIDX:
    std::cout << std::format("{:<16s} R({})[R({})] := R({})\n", opcode_name(op), A, B, C);
    return offset + 1;

  // --- iABC: module ---
  case OpCode::OP_IMPFROM:
    std::cout << std::format("{:<16s} from R({}) import R({})\n", opcode_name(op), A, B);
    return offset + 1;
  case OpCode::OP_IMPALIAS:
    std::cout << std::format("{:<16s} from R({}) import R({}) as R({})\n",
        opcode_name(op), A, B, C);
    return offset + 1;

  // --- iAsBx: iterator ---
  case OpCode::OP_FORITER:
    std::cout << std::format("{:<16s} seq=R({}), idx=R({}), elem=R({}); exit -> {:04d}\n",
        opcode_name(op), A, A + 1, A + 2, static_cast<int>(offset + 1) + sBx);
    return offset + 1;

  // --- Exception handling ---
  case OpCode::OP_TRY:
    std::cout << std::format("{:<16s} handler -> {:04d}\n",
        opcode_name(op), static_cast<int>(offset + 1) + sBx);
    return offset + 1;

  case OpCode::OP_ENDTRY:
  case OpCode::OP_NOP:
    std::cout << std::format("{:<16s}\n", opcode_name(op));
    return offset + 1;

  case OpCode::OP_YIELD:
    std::cout << std::format("{:<16s} R({})\n", opcode_name(op), A);
    return offset + 1;

  case OpCode::OP_RESUME:
    std::cout << std::format("{:<16s} R({}) := resume R({}) send R({})\n",
        opcode_name(op), A, B, C);
    return offset + 1;

  case OpCode::OP_EXTRAARG:
    std::cout << std::format("{:<16s} A={} Bx={}\n", opcode_name(op), A, Bx);
    return offset + 1;

  default:
    std::cout << std::format("Unknown opcode {}\n", static_cast<int>(op));
    return offset + 1;
  }
}

} // namespace ms
