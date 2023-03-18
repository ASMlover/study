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
#include "vm.hh"

namespace clox {

VM::VM() noexcept {
  reset_stack();
}

VM::~VM() noexcept {
}

InterpretResult VM::interpret(Chunk* chunk) noexcept {
  chunk_ = chunk;
  ip_ = chunk->codes();

  return run();
}

InterpretResult VM::interpret(const str_t& source) noexcept {
  // TODO: compile
  return InterpretResult::INTERPRET_OK;
}

InterpretResult VM::run() noexcept {
#define READ_BYTE()     (*ip_++)
#define READ_CONSTANT() (chunk_->get_constant(READ_BYTE()))
#define BINARY_OP(op)\
  do {\
    double b = pop();\
    double a = pop();\
    push(a op b);\
  } while (false)

  for (;;) {
#if defined(_CLOX_DEBUG_TRACE_EXECUTION)
    std::cout << "          ";
    for (Value* slot = stack_; slot < stack_top_; ++slot) {
      std::cout << "[ " << *slot << " ]";
    }
    std::cout << std::endl;
    chunk_->dis_code(as_type<sz_t>(ip_ - chunk_->codes()));
#endif

    switch (OpCode instruction = as_type<OpCode>(READ_BYTE())) {
    case OpCode::OP_CONSTANT:
      {
        Value constant = READ_CONSTANT();
        push(constant);
      } break;
    case OpCode::OP_ADD: BINARY_OP(+); break;
    case OpCode::OP_SUBTRACT: BINARY_OP(-); break;
    case OpCode::OP_MULTIPLY: BINARY_OP(*); break;
    case OpCode::OP_DIVIDE: BINARY_OP(/); break;
    case OpCode::OP_NEGATE: push(-pop()); break;
    case OpCode::OP_RETURN:
      {
        std::cout << pop() << std::endl;
        return InterpretResult::INTERPRET_OK;
      } break;
    }
  }

#undef BINARY_OP
#undef READ_CONSTANT
#undef READ_BYTE
  return InterpretResult::INTERPRET_OK;
}

}
