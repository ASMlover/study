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
#pragma once

#include "Types.hh"

namespace ms {

// =============================================================================
// Register-Based Bytecode: 32-bit fixed-width instructions
//
// Instruction formats (little-endian word layout):
//   iABC:  [C:8][B:8][A:8][Op:8]   — binary ops, most instructions
//   iABx:  [Bx:16][A:8][Op:8]      — constants, globals
//   iAsBx: [sBx:16][A:8][Op:8]     — signed jumps
//
// RK encoding (for B/C fields in arithmetic/comparison ops):
//   Value < 128  → register index R(value)
//   Value >= 128 → constant index K(value - 128), addressing K(0)..K(127)
//   For larger constant pools, use OP_LOADK with Bx field (0..65535).
// =============================================================================

using Instruction = u32_t;

// RK constant threshold: B/C values >= this are constant indices
inline constexpr u8_t kRK_CONST_BIT = 0x80;
inline constexpr int kMAX_RK_CONST = 128;   // K(0)..K(127) via RK encoding
inline constexpr int kMAX_REGISTERS = 128;   // R(0)..R(127)

enum class OpCode : u8_t {
  // --- Loading ---
  OP_LOADK,         // iABx   R(A) := K(Bx)
  OP_LOADNIL,       // iABC   R(A), R(A+1), ..., R(A+B) := nil
  OP_LOADTRUE,      // iA     R(A) := true
  OP_LOADFALSE,     // iA     R(A) := false

  // --- Register movement ---
  OP_MOVE,          // iABC   R(A) := R(B)

  // --- Global variables ---
  OP_GETGLOBAL,     // iABx   R(A) := globals[K(Bx)]
  OP_SETGLOBAL,     // iABx   globals[K(Bx)] := R(A)
  OP_DEFGLOBAL,     // iABx   define globals[K(Bx)] := R(A)

  // --- Upvalues ---
  OP_GETUPVAL,      // iABC   R(A) := upvalues[B]
  OP_SETUPVAL,      // iABC   upvalues[B] := R(A)

  // --- Properties (followed by EXTRAARG for IC slot) ---
  OP_GETPROP,       // iABC   R(A) := R(B).K(C)  [next: EXTRAARG ic_slot]
  OP_SETPROP,       // iABC   R(A).K(B) := R(C)  [next: EXTRAARG ic_slot]
  OP_GETSUPER,      // iABC   R(A) := super(R(B)).K(C)

  // --- Arithmetic (B,C use RK encoding) ---
  OP_ADD,           // iABC   R(A) := RK(B) + RK(C)
  OP_SUB,           // iABC   R(A) := RK(B) - RK(C)
  OP_MUL,           // iABC   R(A) := RK(B) * RK(C)
  OP_DIV,           // iABC   R(A) := RK(B) / RK(C)
  OP_MOD,           // iABC   R(A) := RK(B) % RK(C)

  // --- Comparison (produce bool in register) ---
  OP_EQ,            // iABC   R(A) := (RK(B) == RK(C))
  OP_LT,            // iABC   R(A) := (RK(B) < RK(C))
  OP_LE,            // iABC   R(A) := (RK(B) <= RK(C))

  // --- Unary ---
  OP_NEG,           // iABC   R(A) := -R(B)
  OP_NOT,           // iABC   R(A) := !R(B)
  OP_STR,           // iABC   R(A) := str(R(B))

  // --- Bitwise (B,C use RK encoding) ---
  OP_BAND,          // iABC   R(A) := RK(B) & RK(C)
  OP_BOR,           // iABC   R(A) := RK(B) | RK(C)
  OP_BXOR,          // iABC   R(A) := RK(B) ^ RK(C)
  OP_BNOT,          // iABC   R(A) := ~R(B)
  OP_SHL,           // iABC   R(A) := RK(B) << RK(C)
  OP_SHR,           // iABC   R(A) := RK(B) >> RK(C)

  // --- Control flow ---
  OP_JMP,           // iAsBx  PC += sBx  (A unused)
  OP_TEST,          // iABC   if (bool(R(A)) != C) then PC++
  OP_TESTSET,       // iABC   if (bool(R(B)) == C) R(A):=R(B) else PC++

  // --- I/O ---
  OP_PRINT,         // iA     print R(A)

  // --- Function calls ---
  OP_CALL,          // iABC   R(A)..R(A+C-2) := R(A)(R(A+1)..R(A+B-1))
  OP_INVOKE,        // iABC   A=base, B=argc, C=name_K  [next: EXTRAARG ic]
  OP_SUPERINV,      // iABC   A=base, B=argc, C=name_K
  OP_RETURN,        // iABC   return R(A)..R(A+B-2)  (B=1: return R(A))
  OP_CLOSURE,       // iABx   R(A) := closure(K(Bx))  [+upvalue EXTRAARGs]
  OP_CLOSE,         // iA     close all upvalues >= R(A)

  // --- OOP ---
  OP_CLASS,         // iABx   R(A) := new class(K(Bx))
  OP_INHERIT,       // iABC   R(A).inherit(R(B))
  OP_METHOD,        // iABC   class(R(A)).methods[K(B)] := R(C)
  OP_STATICMETH,    // iABC   class(R(A)).static[K(B)] := R(C)
  OP_GETTER,        // iABC   class(R(A)).getters[K(B)] := R(C)
  OP_SETTER,        // iABC   class(R(A)).setters[K(B)] := R(C)
  OP_ABSTMETH,      // iABC   class(R(A)).abstract[K(B)] := R(C)

  // --- Collections ---
  OP_NEWLIST,       // iABC   R(A) := [R(A+1)..R(A+B)]
  OP_NEWMAP,        // iABC   R(A) := {R(A+1):R(A+2), ..., R(A+2B-1):R(A+2B)}
  OP_NEWTUPLE,      // iABC   R(A) := (R(A+1)..R(A+B))
  OP_GETIDX,        // iABC   R(A) := R(B)[R(C)]
  OP_SETIDX,        // iABC   R(A)[R(B)] := R(C)

  // --- Module ---
  OP_IMPORT,        // iA     import path from R(A)
  OP_IMPFROM,       // iABC   from R(A) import R(B)
  OP_IMPALIAS,      // iABC   from R(A) import R(B) as R(C)

  // --- Iterator ---
  OP_FORITER,       // iABx   seq=R(A), idx=R(A+1), elem→R(A+2); sBx=exit offset

  // --- Exception handling ---
  OP_THROW,         // iA     throw R(A)
  OP_TRY,           // iAsBx  push handler at PC+sBx
  OP_ENDTRY,        // i      pop exception handler
  OP_DEFER,         // iA     defer closure R(A)

  // --- Extra data ---
  OP_EXTRAARG,      // iABx   extra data word for preceding instruction
};

// =============================================================================
// Instruction encoding
// =============================================================================

// iABC format: [C:8][B:8][A:8][Op:8]
inline Instruction encode_ABC(OpCode op, u8_t a, u8_t b, u8_t c) noexcept {
  return static_cast<u32_t>(op)
       | (static_cast<u32_t>(a) << 8)
       | (static_cast<u32_t>(b) << 16)
       | (static_cast<u32_t>(c) << 24);
}

// iABx format: [Bx:16][A:8][Op:8]
inline Instruction encode_ABx(OpCode op, u8_t a, u16_t bx) noexcept {
  return static_cast<u32_t>(op)
       | (static_cast<u32_t>(a) << 8)
       | (static_cast<u32_t>(bx) << 16);
}

// iAsBx format: [sBx:16][A:8][Op:8]  — sBx biased by 32767
inline constexpr int kSBX_BIAS = 32767;

inline Instruction encode_AsBx(OpCode op, u8_t a, int sbx) noexcept {
  auto bx = static_cast<u16_t>(sbx + kSBX_BIAS);
  return encode_ABx(op, a, bx);
}

// =============================================================================
// Instruction decoding
// =============================================================================

inline OpCode decode_op(Instruction i) noexcept {
  return static_cast<OpCode>(i & 0xFF);
}

inline u8_t decode_A(Instruction i) noexcept {
  return static_cast<u8_t>((i >> 8) & 0xFF);
}

inline u8_t decode_B(Instruction i) noexcept {
  return static_cast<u8_t>((i >> 16) & 0xFF);
}

inline u8_t decode_C(Instruction i) noexcept {
  return static_cast<u8_t>((i >> 24) & 0xFF);
}

inline u16_t decode_Bx(Instruction i) noexcept {
  return static_cast<u16_t>((i >> 16) & 0xFFFF);
}

inline int decode_sBx(Instruction i) noexcept {
  return static_cast<int>(decode_Bx(i)) - kSBX_BIAS;
}

// =============================================================================
// RK helpers: Register-or-Constant encoding for B/C fields
// =============================================================================

inline constexpr bool is_rk_const(u8_t rk) noexcept {
  return (rk & kRK_CONST_BIT) != 0;
}

inline constexpr u8_t rk_to_const(u8_t rk) noexcept {
  return rk & ~kRK_CONST_BIT;
}

inline constexpr u8_t const_to_rk(u8_t k_index) noexcept {
  return k_index | kRK_CONST_BIT;
}

// =============================================================================
// Opcode names (for disassembly / debug)
// =============================================================================

inline cstr_t opcode_name(OpCode code) noexcept {
  switch (code) {
  case OpCode::OP_LOADK:       return "LOADK";
  case OpCode::OP_LOADNIL:     return "LOADNIL";
  case OpCode::OP_LOADTRUE:    return "LOADTRUE";
  case OpCode::OP_LOADFALSE:   return "LOADFALSE";
  case OpCode::OP_MOVE:        return "MOVE";
  case OpCode::OP_GETGLOBAL:   return "GETGLOBAL";
  case OpCode::OP_SETGLOBAL:   return "SETGLOBAL";
  case OpCode::OP_DEFGLOBAL:   return "DEFGLOBAL";
  case OpCode::OP_GETUPVAL:    return "GETUPVAL";
  case OpCode::OP_SETUPVAL:    return "SETUPVAL";
  case OpCode::OP_GETPROP:     return "GETPROP";
  case OpCode::OP_SETPROP:     return "SETPROP";
  case OpCode::OP_GETSUPER:    return "GETSUPER";
  case OpCode::OP_ADD:         return "ADD";
  case OpCode::OP_SUB:         return "SUB";
  case OpCode::OP_MUL:         return "MUL";
  case OpCode::OP_DIV:         return "DIV";
  case OpCode::OP_MOD:         return "MOD";
  case OpCode::OP_EQ:          return "EQ";
  case OpCode::OP_LT:          return "LT";
  case OpCode::OP_LE:          return "LE";
  case OpCode::OP_NEG:         return "NEG";
  case OpCode::OP_NOT:         return "NOT";
  case OpCode::OP_STR:         return "STR";
  case OpCode::OP_BAND:        return "BAND";
  case OpCode::OP_BOR:         return "BOR";
  case OpCode::OP_BXOR:        return "BXOR";
  case OpCode::OP_BNOT:        return "BNOT";
  case OpCode::OP_SHL:         return "SHL";
  case OpCode::OP_SHR:         return "SHR";
  case OpCode::OP_JMP:         return "JMP";
  case OpCode::OP_TEST:        return "TEST";
  case OpCode::OP_TESTSET:     return "TESTSET";
  case OpCode::OP_PRINT:       return "PRINT";
  case OpCode::OP_CALL:        return "CALL";
  case OpCode::OP_INVOKE:      return "INVOKE";
  case OpCode::OP_SUPERINV:    return "SUPERINV";
  case OpCode::OP_RETURN:      return "RETURN";
  case OpCode::OP_CLOSURE:     return "CLOSURE";
  case OpCode::OP_CLOSE:       return "CLOSE";
  case OpCode::OP_CLASS:       return "CLASS";
  case OpCode::OP_INHERIT:     return "INHERIT";
  case OpCode::OP_METHOD:      return "METHOD";
  case OpCode::OP_STATICMETH:  return "STATICMETH";
  case OpCode::OP_GETTER:      return "GETTER";
  case OpCode::OP_SETTER:      return "SETTER";
  case OpCode::OP_ABSTMETH:    return "ABSTMETH";
  case OpCode::OP_NEWLIST:     return "NEWLIST";
  case OpCode::OP_NEWMAP:      return "NEWMAP";
  case OpCode::OP_NEWTUPLE:    return "NEWTUPLE";
  case OpCode::OP_GETIDX:      return "GETIDX";
  case OpCode::OP_SETIDX:      return "SETIDX";
  case OpCode::OP_IMPORT:      return "IMPORT";
  case OpCode::OP_IMPFROM:     return "IMPFROM";
  case OpCode::OP_IMPALIAS:    return "IMPALIAS";
  case OpCode::OP_FORITER:     return "FORITER";
  case OpCode::OP_THROW:       return "THROW";
  case OpCode::OP_TRY:         return "TRY";
  case OpCode::OP_ENDTRY:      return "ENDTRY";
  case OpCode::OP_DEFER:       return "DEFER";
  case OpCode::OP_EXTRAARG:    return "EXTRAARG";
  default:                     return "UNKNOWN";
  }
}

} // namespace ms
