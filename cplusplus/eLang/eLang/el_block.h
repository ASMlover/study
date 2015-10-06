// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_BLOCK_HEADER_H__
#define __EL_BLOCK_HEADER_H__

#include "el_object.h"

#define DECODE_OP(inst) (static_cast<OpCode>((inst & 0xff000000) >> 24))
#define DECODE_A(inst)  ((inst & 0x00ff0000) >> 16)
#define DECODE_B(inst)  ((inst & 0x0000ff00) >> 8)
#define DECODE_C(inst)  (inst & 0x000000ff)

namespace el {

typedef unsigned int Instruction;

enum class OpCode {
  OP_CONSTANT,      // A = index of constant, B = dest register
  OP_BLOCK,         // A = index of example, B = dest register
  OP_OBJECT,        // A = parent register and dest register
  OP_ARRAY,         // A = initial capacity, B = dest register
  OP_ARRAY_ELEMENT, // A = element register, B = dest array register
  OP_MOVE,          // A = source register, B = dest register
  OP_SELF,          // A = dest register
  OP_MESSAGE_0,     // A = index of message name in string table
  OP_MESSAGE_1,     // B = register of receiver (args follow)
  OP_MESSAGE_2,     // C = dest register
  OP_MESSAGE_3,
  OP_MESSAGE_4,
  OP_MESSAGE_5,
  OP_MESSAGE_6,
  OP_MESSAGE_7,
  OP_MESSAGE_8,
  OP_MESSAGE_9,
  OP_MESSAGE_10,
  OP_TAIL_MESSAGE_0,
  OP_TAIL_MESSAGE_1,
  OP_TAIL_MESSAGE_2,
  OP_TAIL_MESSAGE_3,
  OP_TAIL_MESSAGE_4,
  OP_TAIL_MESSAGE_5,
  OP_TAIL_MESSAGE_6,
  OP_TAIL_MESSAGE_7,
  OP_TAIL_MESSAGE_8,
  OP_TAIL_MESSAGE_9,
  OP_TAIL_MESSAGE_10,
  OP_GET_UPVALUE,   // A = index of upvalue, B = dest register
  OP_SET_UPVALUE,   // A = index of upvalue, B = value register
  OP_GET_FIELD,     // A = index of field in string table, B = dest register
  OP_SET_FIELD,     // A = index of field in string table, B = value register
  OP_GET_GLOBAL,    // A = index of global, B = dest register
  OP_SET_GLOBAL,    // A = index of global, B = value register
  OP_DEF_METHOD,    // A = index of method name in string table,
                    // B = index of method body block,
                    // C = object method is being defined on
  OP_DEF_FIELD,     // A = index of field name in string table,
                    // B = register with field value,
                    // C = object field is being defined on
  OP_END,           // A = register with result to return
  OP_RETURN,        // A = method id to return from,
                    // B = register with value to return
  OP_CAPTURE_LOCAL,   // A = register of local
  OP_CAPTURE_UPVALUE, // A = index of upvalue
};

class Block {
  int                 method_id_;
  Array<String>       params_;
  Array<Instruction>  code_;
  Array<Value>        constants_;
  Array<Ref<Block> >  blocks_;
  int                 num_registers_;
  int                 num_upvalues_;
};

}

#endif  // __EL_BLOCK_HEADER_H__
