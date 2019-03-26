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
#include <cstdio>
#include <iostream>
#include "chunk.hh"
#include "compiler.hh"
#include "vm.hh"

namespace lvm {

static VM* _running_vm = nullptr;

VM* get_running_vm(void) {
  return _running_vm;
}

VM::VM(Chunk& c)
  : chunk_(c) {
  _running_vm = this;
}

VM::~VM(void) {
  for (auto* o : objects_)
    delete o;
  objects_.clear();
}

InterpretRet VM::interpret(void) {
  ip_ = chunk_.get_codes();
  return run();
}

InterpretRet VM::interpret(const std::string& source_bytes) {
  Compiler c;
  Chunk chunk;
  stack_.clear();
  if (!c.compile(chunk, source_bytes))
    return InterpretRet::COMPILE_ERROR;

  if (!chunk.is_valid())
    return InterpretRet::OK;

  chunk_ = chunk;
  ip_ = chunk_.get_codes();
  return run();
}

void VM::put_in(Object* o) {
  objects_.push_back(o);
}

void VM::set_interned_string(std::uint32_t c, StringObject* s) {
  interned_strings_[c] = s;
}

StringObject* VM::get_interned_string(std::uint32_t c) const {
  auto intern_iter = interned_strings_.find(c);
  return intern_iter != interned_strings_.end() ? intern_iter->second : nullptr;
}

InterpretRet VM::run(void) {
  auto _rdbyte = [this](void) -> OpCode {
    return *ip_++;
  };
  auto _rdconstant = [this, _rdbyte](void) -> Value {
    return chunk_.get_constant(EnumUtil<OpCode>::as_int(_rdbyte()));
  };
  auto _rdstring = [this, _rdconstant](void) -> StringObject* {
    return dynamic_cast<StringObject*>(_rdconstant().as_object());
  };

#define BINARY_OP(op) do {\
  if (!peek(0).is_numeric() || !peek(1).is_numeric()) {\
    runtime_error("operands must be numerics");\
    return InterpretRet::RUNTIME_ERROR;\
  }\
  Value b = pop();\
  Value a = pop();\
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
    case OpCode::OP_NIL: push(nullptr); break;
    case OpCode::OP_TRUE: push(true); break;
    case OpCode::OP_FALSE: push(false); break;
    case OpCode::OP_POP: pop(); break;
    case OpCode::OP_DEFINE_GLOBAL:
      {
        StringObject* name = _rdstring();
        global_variables_[name->hash_code()] = peek(0);
        pop();
      } break;
    case OpCode::OP_EQ:
      {
        Value b = pop();
        Value a = pop();
        push(a == b);
      } break;
    case OpCode::OP_NE:
      {
        Value b = pop();
        Value a = pop();
        push(a != b);
      } break;
    case OpCode::OP_GT: BINARY_OP(>); break;
    case OpCode::OP_GE: BINARY_OP(>=); break;
    case OpCode::OP_LT: BINARY_OP(<); break;
    case OpCode::OP_LE: BINARY_OP(<=); break;
    case OpCode::OP_ADD:
      {
        if (peek(0).is_string() && peek(1).is_string()) {
          Value b = pop();
          Value a = pop();
          push(Object::concat_string(a.as_object(), b.as_object()));
        }
        else if (peek(0).is_numeric() && peek(1).is_numeric()) {
          Value b = pop();
          Value a = pop();
          push(a + b);
        }
        else {
          runtime_error("operands must be two numerics and two strings");
          return InterpretRet::RUNTIME_ERROR;
        }
      } break;
    case OpCode::OP_SUB: BINARY_OP(-); break;
    case OpCode::OP_MUL: BINARY_OP(*); break;
    case OpCode::OP_DIV: BINARY_OP(/); break;
    case OpCode::OP_NOT: push(!pop()); break;
    case OpCode::OP_NEGATE:
      if (!peek().is_numeric()) {
        runtime_error("operand must be a numeric");
        return InterpretRet::RUNTIME_ERROR;
      }
      push(-pop()); break;
    case OpCode::OP_PRINT:
      std::cout << pop() << std::endl; break;
    case OpCode::OP_RETURN:
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

Value VM::peek(int distance) const {
  return stack_[stack_.size() - 1 - distance];
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  std::vfprintf(stderr, format, ap);
  va_end(ap);
  std::cerr << std::endl;

  int offset = static_cast<int>(ip_ - chunk_.get_codes());
  std::cerr << "line(" << chunk_.get_line(offset) << ") in script" << std::endl;
}

}
