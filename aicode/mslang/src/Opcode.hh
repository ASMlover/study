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

enum class OpCode : u8_t {
  OP_CONSTANT,
  OP_CONSTANT_LONG,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_UPVALUE,
  OP_SET_UPVALUE,
  OP_GET_PROPERTY,
  OP_SET_PROPERTY,
  OP_GET_SUPER,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MODULO,
  OP_NOT,
  OP_NEGATE,
  OP_STR,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP,
  OP_CALL,
  OP_INVOKE,
  OP_SUPER_INVOKE,
  OP_CLOSURE,
  OP_CLOSE_UPVALUE,
  OP_RETURN,
  OP_CLASS,
  OP_INHERIT,
  OP_METHOD,
  OP_STATIC_METHOD,
  OP_GETTER,
  OP_SETTER,
  OP_BUILD_LIST,
  OP_BUILD_MAP,
  OP_INDEX_GET,
  OP_INDEX_SET,
  OP_IMPORT,
  OP_IMPORT_FROM,
  OP_IMPORT_ALIAS,
  OP_FOR_ITER,
  OP_THROW,
  OP_TRY,
  OP_END_TRY,
  OP_DEFER,

  // Superinstructions: fused GET_LOCAL+GET_LOCAL+binary_op
  OP_ADD_LOCAL_LOCAL,       // slot1, slot2 → push(locals[slot1] + locals[slot2])
  OP_SUBTRACT_LOCAL_LOCAL,  // slot1, slot2 → push(locals[slot1] - locals[slot2])
  OP_MULTIPLY_LOCAL_LOCAL,  // slot1, slot2 → push(locals[slot1] * locals[slot2])
  OP_DIVIDE_LOCAL_LOCAL,    // slot1, slot2 → push(locals[slot1] / locals[slot2])
  OP_MODULO_LOCAL_LOCAL,    // slot1, slot2 → push(locals[slot1] % locals[slot2])
};

inline cstr_t opcode_name(OpCode code) noexcept {
  switch (code) {
  case OpCode::OP_CONSTANT:       return "OP_CONSTANT";
  case OpCode::OP_CONSTANT_LONG:  return "OP_CONSTANT_LONG";
  case OpCode::OP_NIL:            return "OP_NIL";
  case OpCode::OP_TRUE:           return "OP_TRUE";
  case OpCode::OP_FALSE:          return "OP_FALSE";
  case OpCode::OP_POP:            return "OP_POP";
  case OpCode::OP_GET_LOCAL:      return "OP_GET_LOCAL";
  case OpCode::OP_SET_LOCAL:      return "OP_SET_LOCAL";
  case OpCode::OP_GET_GLOBAL:     return "OP_GET_GLOBAL";
  case OpCode::OP_DEFINE_GLOBAL:  return "OP_DEFINE_GLOBAL";
  case OpCode::OP_SET_GLOBAL:     return "OP_SET_GLOBAL";
  case OpCode::OP_GET_UPVALUE:    return "OP_GET_UPVALUE";
  case OpCode::OP_SET_UPVALUE:    return "OP_SET_UPVALUE";
  case OpCode::OP_GET_PROPERTY:   return "OP_GET_PROPERTY";
  case OpCode::OP_SET_PROPERTY:   return "OP_SET_PROPERTY";
  case OpCode::OP_GET_SUPER:      return "OP_GET_SUPER";
  case OpCode::OP_EQUAL:          return "OP_EQUAL";
  case OpCode::OP_GREATER:        return "OP_GREATER";
  case OpCode::OP_LESS:           return "OP_LESS";
  case OpCode::OP_ADD:            return "OP_ADD";
  case OpCode::OP_SUBTRACT:       return "OP_SUBTRACT";
  case OpCode::OP_MULTIPLY:       return "OP_MULTIPLY";
  case OpCode::OP_DIVIDE:         return "OP_DIVIDE";
  case OpCode::OP_MODULO:        return "OP_MODULO";
  case OpCode::OP_NOT:            return "OP_NOT";
  case OpCode::OP_NEGATE:         return "OP_NEGATE";
  case OpCode::OP_STR:            return "OP_STR";
  case OpCode::OP_PRINT:          return "OP_PRINT";
  case OpCode::OP_JUMP:           return "OP_JUMP";
  case OpCode::OP_JUMP_IF_FALSE:  return "OP_JUMP_IF_FALSE";
  case OpCode::OP_LOOP:           return "OP_LOOP";
  case OpCode::OP_CALL:           return "OP_CALL";
  case OpCode::OP_INVOKE:         return "OP_INVOKE";
  case OpCode::OP_SUPER_INVOKE:   return "OP_SUPER_INVOKE";
  case OpCode::OP_CLOSURE:        return "OP_CLOSURE";
  case OpCode::OP_CLOSE_UPVALUE:  return "OP_CLOSE_UPVALUE";
  case OpCode::OP_RETURN:         return "OP_RETURN";
  case OpCode::OP_CLASS:          return "OP_CLASS";
  case OpCode::OP_INHERIT:        return "OP_INHERIT";
  case OpCode::OP_METHOD:         return "OP_METHOD";
  case OpCode::OP_STATIC_METHOD:  return "OP_STATIC_METHOD";
  case OpCode::OP_GETTER:         return "OP_GETTER";
  case OpCode::OP_SETTER:         return "OP_SETTER";
  case OpCode::OP_BUILD_LIST:     return "OP_BUILD_LIST";
  case OpCode::OP_BUILD_MAP:      return "OP_BUILD_MAP";
  case OpCode::OP_INDEX_GET:      return "OP_INDEX_GET";
  case OpCode::OP_INDEX_SET:      return "OP_INDEX_SET";
  case OpCode::OP_IMPORT:         return "OP_IMPORT";
  case OpCode::OP_IMPORT_FROM:    return "OP_IMPORT_FROM";
  case OpCode::OP_IMPORT_ALIAS:   return "OP_IMPORT_ALIAS";
  case OpCode::OP_FOR_ITER:       return "OP_FOR_ITER";
  case OpCode::OP_THROW:          return "OP_THROW";
  case OpCode::OP_TRY:            return "OP_TRY";
  case OpCode::OP_END_TRY:        return "OP_END_TRY";
  case OpCode::OP_DEFER:               return "OP_DEFER";
  case OpCode::OP_ADD_LOCAL_LOCAL:      return "OP_ADD_LOCAL_LOCAL";
  case OpCode::OP_SUBTRACT_LOCAL_LOCAL: return "OP_SUBTRACT_LOCAL_LOCAL";
  case OpCode::OP_MULTIPLY_LOCAL_LOCAL: return "OP_MULTIPLY_LOCAL_LOCAL";
  case OpCode::OP_DIVIDE_LOCAL_LOCAL:   return "OP_DIVIDE_LOCAL_LOCAL";
  case OpCode::OP_MODULO_LOCAL_LOCAL:   return "OP_MODULO_LOCAL_LOCAL";
  default:                              return "OP_UNKNOWN";
  }
}

} // namespace ms
