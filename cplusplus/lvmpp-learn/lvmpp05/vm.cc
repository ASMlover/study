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

InterpretRet VM::run(void) {
  auto _rdbyte = [this](void) -> OpCode {
    return *ip_++;
  };
  auto _rdconstant = [this, _rdbyte](void) -> Value {
    return chunk_.get_constant(EnumUtil<OpCode>::as_int(_rdbyte()));
  };

  for (;;) {
#if defined(LVM_TRACE_EXECUTION)
    int offset = static_cast<int>(ip_ - chunk_.get_codes());
    chunk_.disassemble_instruction(offset);
#endif

    OpCode instruction = _rdbyte();
    switch (instruction) {
    case OpCode::OP_CONSTANT:
      {
        Value constant = _rdconstant();
        std::cout << constant << std::endl;
      } break;
    case OpCode::OP_RETURN:
      return InterpretRet::OK;
    }
  }

  return InterpretRet::OK;
}

}
