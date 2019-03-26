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
#include "chunk.hh"
#include "compiler.hh"
#include "vm.hh"

namespace lvm {

VM::VM(Chunk& c)
  : chunk_(c) {
}

VM::~VM(void) {
}

InterpretRet VM::interpret(void) {
  ip_ = chunk_.get_codes();
  return run();
}

InterpretRet VM::interpret(const std::string& source_bytes) {
  Compiler c;
  c.compile(source_bytes);
  return InterpretRet::OK;
}

InterpretRet VM::run(void) {
  auto _rdbyte = [this](void) -> OpCode {
    return *ip_++;
  };
  auto _rdconstant = [this, _rdbyte](void) -> Value {
    return chunk_.get_constant(EnumUtil<OpCode>::as_int(_rdbyte()));
  };

#define BINARY_OP(op) do {\
  double b = pop();\
  double a = pop();\
  push(a op b);\
} while (false)

  for (;;) {
#if defined(LVM_TRACE_EXECUTION)
    std::cout << "          ";
    for (auto& v : stack_)
      std::cout << "[" << v << "]";
    std::cout << std::endl;
    int offset = static_cast<int>(ip_ - chunk_.get_codes());
    chunk_.disassemble_instruction(offset);
#endif

    OpCode instruction = _rdbyte();
    switch (instruction) {
    case OpCode::OP_CONSTANT: push(_rdconstant()); break;
    case OpCode::OP_ADD: BINARY_OP(+); break;
    case OpCode::OP_SUB: BINARY_OP(-); break;
    case OpCode::OP_MUL: BINARY_OP(*); break;
    case OpCode::OP_DIV: BINARY_OP(/); break;
    case OpCode::OP_NEGATE: push(-pop()); break;
    case OpCode::OP_RETURN:
      {
        std::cout << pop() << std::endl;
      }
      return InterpretRet::OK;
    }
  }
  return InterpretRet::OK;

#undef BINARY_OP
}

void VM::reset_stack(void) {
  stack_.clear();
}

void VM::push(const Value& value) {
  stack_.push_back(value);
}

Value VM::pop(void) {
  auto value = stack_.back();
  stack_.pop_back();
  return value;
}

}
