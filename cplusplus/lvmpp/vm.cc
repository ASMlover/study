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
#include "chunk.h"
#include "vm.h"

namespace lox {

InterpretRet VM::interpret(void) {
  ip_ = 0;
  return run();
}

InterpretRet VM::run(void) {
  auto _rdbyte = [this]() -> std::uint8_t {
    return chunk_.get_code(ip_++);
  };
  auto _rdconstant = [this, _rdbyte]() -> Value {
    return chunk_.get_constant(_rdbyte());
  };

  for (;;) {
#if defined(DEBUG_TRACE_EXECUTION)
    std::cout << "          ";
    for (auto& v : stack_)
      std::cout << "[ " << v << " ]";
    std::cout << std::endl;
    chunk_.disassemble_instruction(ip_);
#endif

    std::uint8_t instruction = _rdbyte();
    switch (instruction) {
    case OpCode::OP_CONSTANT:
      {
        Value constant = _rdconstant();
        push(constant);
      } break;
    case OpCode::OP_RETURN:
      std::cout << pop() << std::endl;
      return InterpretRet::OK;
    }
  }
  return InterpretRet::OK;
}

}
