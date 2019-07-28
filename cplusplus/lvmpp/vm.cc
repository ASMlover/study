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
#include <cstdarg>
#include <iostream>
#include "object.h"
#include "compiler.h"
#include "chunk.h"
#include "vm.h"

namespace lox {

static VM* _vm_object = nullptr;

VM* global_vm(void) {
  return _vm_object;
}

VM::VM(Chunk& c)
  : chunk_(c) {
  stack_.reserve(kDefaultCap);
  _vm_object = this;
}

VM::~VM(void) {
  for (auto* o : objects_)
    delete o;
  objects_.clear();
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  std::vfprintf(stderr, format, ap);
  va_end(ap);
  std::cerr << std::endl;

  std::cerr << "line(" << chunk_.get_line(ip_) << ") in script" << std::endl;
}

StringObject* VM::fetch_out(std::uint32_t code) const {
  auto str_iter = strings_.find(code);
  return str_iter != strings_.end() ? str_iter->second : nullptr;
}

InterpretRet VM::interpret(void) {
  ip_ = 0;
  return run();
}

InterpretRet VM::run(void) {
  auto _rdbyte = [this]() -> std::uint8_t {
    return chunk_.get_code(ip_++);
  };
  auto _rdshort = [this]() -> std::uint8_t {
    return ip_ += 2, static_cast<std::uint8_t>(
        (chunk_.get_code(ip_ - 2) << 8) | chunk_.get_code(ip_ - 1));
  };
  auto _rdconstant = [this, _rdbyte]() -> Value {
    return chunk_.get_constant(_rdbyte());
  };
  auto _rdstring = [this, _rdconstant]() -> StringObject* {
    return dynamic_cast<StringObject*>(_rdconstant().as_object());
  };

#define BINARY_OP(op) do {\
  if (!peek(0).is_numeric() || !peek(1).is_numeric()) {\
    runtime_error("operands must be numerics ...");\
    return InterpretRet::RUNTIME_ERROR;\
  }\
  double b = pop();\
  double a = pop();\
  push(a op b);\
} while (false)

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
    case OpCode::OP_CONSTANT: {
      Value constant = _rdconstant();
      push(constant);
    } break;
    case OpCode::OP_NEGATIVE:
      if (!peek(0).is_numeric()) {
        runtime_error("operand must be a numeric ...");
        return InterpretRet::RUNTIME_ERROR;
      }
      push(-pop()); break;
    case OpCode::OP_NOT: push(!pop().is_truthy()); break;
    case OpCode::OP_NIL: push(nullptr); break;
    case OpCode::OP_TRUE: push(true); break;
    case OpCode::OP_FALSE: push(false); break;
    case OpCode::OP_POP: pop(); break;
    case OpCode::OP_GET_LOCAL: {
      auto slot = _rdbyte();
      push(stack_[slot]);
    } break;
    case OpCode::OP_SET_LOCAL: {
      auto slot = _rdbyte();
      stack_[slot] = peek(0);
    } break;
    case OpCode::OP_GET_GLOBAL: {
      StringObject* name = _rdstring();
      auto value_iter = globals_.find(name->hash_code());
      if (value_iter ==  globals_.end()) {
        runtime_error("undefined variable `%s` ...", name->c_str());
        return InterpretRet::RUNTIME_ERROR;
      }
      push(value_iter->second);
    } break;
    case OpCode::OP_SET_GLOBAL: {
      StringObject* name = _rdstring();
      auto value_iter = globals_.find(name->hash_code());
      if (value_iter == globals_.end()) {
        runtime_error("undefined variable `%s` ...", name->c_str());
        return InterpretRet::RUNTIME_ERROR;
      }
      globals_[name->hash_code()] = peek(0);
    } break;
    case OpCode::OP_DEFINE_GLOBAL: {
      StringObject* name = _rdstring();
      globals_[name->hash_code()] = peek(0);
      pop();
    } break;
    case OpCode::OP_EQUAL: {
      Value b = pop();
      Value a = pop();
      push(a == b);
    } break;
    case OpCode::OP_GREATER: BINARY_OP(>); break;
    case OpCode::OP_LESS: BINARY_OP(<); break;
    case OpCode::OP_ADD:
      if (peek(0).is_object(ObjType::STRING)
          && peek(1).is_object(ObjType::STRING)) {
        Value b = pop();
        Value a = pop();
        push(concat(a.as_object(), b.as_object()));
      }
      else if (peek(0).is_numeric() && peek(1).is_numeric()) {
        double b = pop();
        double a = pop();
        push(a + b);
      }
      else {
        runtime_error("operands must be two numbers or two strings ...");
        return InterpretRet::RUNTIME_ERROR;
      }
      break;
    case OpCode::OP_SUBTRACT: BINARY_OP(-); break;
    case OpCode::OP_MULTIPLY: BINARY_OP(*); break;
    case OpCode::OP_DIVIDE: BINARY_OP(/); break;
    case OpCode::OP_PRINT:
      std::cout << pop() << std::endl;
      break;
    case OpCode::OP_JUMP_IF_FALSE:
      {
        std::uint16_t offset = _rdshort();
        if (!peek(0).is_truthy())
          ip_ += offset;
      } break;
    case OpCode::OP_RETURN:
      return InterpretRet::OK;
    }
  }
  return InterpretRet::OK;

#undef BINARY_MOD
#undef BINARY_OP
}

InterpretRet VM::interpret(const std::string& source_bytes) {
  Chunk chunk;
  Compiler c;
  stack_.clear();
  if (!c.compile(chunk, source_bytes))
    return InterpretRet::COMPILE_ERROR;

  chunk_ = chunk;
  ip_ = 0;
  return run();
}

}
