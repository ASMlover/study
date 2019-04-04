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
#include <cassert>
#include <iostream>
#include <sstream>
#include "compile.hh"
#include "vm.hh"

namespace nyx {

VM::VM(void) {
  globals_ = TableObject::create(*this);
}

VM::~VM(void) {
  globals_ = nullptr;

  for (auto* o : objects_)
    free_object(o);
  objects_.clear();
  gray_stack_.clear();
}

bool VM::pop_boolean(bool& v) {
  if (peek()->type() != ObjType::BOOLEAN) {
    std::cerr << "operand must be a boolean" << std::endl;
    return false;
  }

  v = Xptr::down<BooleanObject>(pop())->value();
  return true;
}

bool VM::pop_numeric(double& v) {
  if (peek()->type() != ObjType::NUMERIC) {
    std::cerr << "operand must be a numeric" << std::endl;
    return false;
  }

  v = Xptr::down<NumericObject>(pop())->value();
  return true;
}

bool VM::pop_numerics(double& a, double& b) {
  if (peek(0)->type() != ObjType::NUMERIC) {
    std::cerr << "right operand must be a numeric" << std::endl;
    return false;
  }
  if (peek(1)->type() != ObjType::NUMERIC) {
    std::cerr << "left operand must be a numeric" << std::endl;
    return false;
  }

  b = Xptr::down<NumericObject>(pop())->value();
  a = Xptr::down<NumericObject>(pop())->value();
  return true;
}

void VM::collect(void) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "********* collect: starting *********" << std::endl;
#endif

  // mark the roots
  for (auto* v : stack_)
    gray_value(v);

  gray_value(globals_);
  gray_compiler_roots();

  while (!gray_stack_.empty()) {
    auto* o = gray_stack_.back();
    gray_stack_.pop_back();
    blacken_object(o);
  }

  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->is_dark()) {
      free_object(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_dark(false);
      ++it;
    }
  }

#if defined(DEBUG_GC_TRACE)
  std::cout << "********* collect: finished *********" << std::endl;
#endif
}

void VM::print_stack(void) {
  int i{};
  for (auto* o : stack_)
    std::cout << i++ << " : " << o << std::endl;
}

void VM::run(FunctionObject* fn) {
  push(fn);
  const u8_t* ip = fn->codes();
  for (;;) {
    switch (*ip++) {
    case OpCode::OP_CONSTANT:
      {
        u8_t constant = *ip++;
        push(fn->get_constant(constant));
      } break;
    case OpCode::OP_DEF_GLOBAL:
      {
        u8_t constant = *ip++;
        auto* key = Xptr::down<StringObject>(fn->get_constant(constant));
        globals_->set_entry(key, pop());
      } break;
    case OpCode::OP_GET_GLOBAL:
      {
        u8_t constant = *ip++;
        auto* key = Xptr::down<StringObject>(fn->get_constant(constant));
        push(globals_->get_entry(key));
      } break;
    case OpCode::OP_SET_GLOBAL:
      {
        u8_t constant = *ip++;
        auto* key = Xptr::down<StringObject>(fn->get_constant(constant));
        globals_->set_entry(key, pop());
      } break;
    case OpCode::OP_GT:
      {
        double a, b;
        if (!pop_numerics(a, b))
          return;
        push(BooleanObject::create(*this, a > b));
      } break;
    case OpCode::OP_GE:
      {
        double a, b;
        if (!pop_numerics(a, b))
          return;
        push(BooleanObject::create(*this, a >= b));
      } break;
    case OpCode::OP_LT:
      {
        double a, b;
        if (!pop_numerics(a, b))
          return;
        push(BooleanObject::create(*this, a < b));
      } break;
    case OpCode::OP_LE:
      {
        double a, b;
        if (!pop_numerics(a, b))
          return;
        push(BooleanObject::create(*this, a <= b));
      } break;
    case OpCode::OP_ADD:
      {
        if (peek(0)->type() == ObjType::STRING
            && peek(1)->type() == ObjType::STRING) {
          auto* b = Xptr::down<StringObject>(pop());
          auto* a = Xptr::down<StringObject>(pop());
          push(StringObject::concat(*this, a, b));
        }
        else if (peek(0)->type() == ObjType::NUMERIC
            && peek(1)->type() == ObjType::NUMERIC) {
          double b = Xptr::down<NumericObject>(pop())->value();
          double a = Xptr::down<NumericObject>(pop())->value();
          push(NumericObject::create(*this, a + b));
        }
        else {
          std::cerr << "can only add two strings or two numerics" << std::endl;
          return;
        }
      } break;
    case OpCode::OP_SUB:
      {
        double a, b;
        if (!pop_numerics(a, b))
          return;
        push(NumericObject::create(*this, a - b));
      } break;
    case OpCode::OP_MUL:
      {
        double a, b;
        if (!pop_numerics(a, b))
          return;
        push(NumericObject::create(*this, a * b));
      } break;
    case OpCode::OP_DIV:
      {
        double a, b;
        if (!pop_numerics(a, b))
          return;
        push(NumericObject::create(*this, a / b));
      } break;
    case OpCode::OP_NOT:
      {
        bool v;
        if (!pop_boolean(v))
          return;
        push(BooleanObject::create(*this, !v));
      } break;
    case OpCode::OP_NEG:
      {
        double v;
        if (!pop_numeric(v))
          return;
        push(NumericObject::create(*this, -v));
      } break;
    case OpCode::OP_RETURN:
      // std::cout << stack_.back() << std::endl;
      std::cout << pop() << std::endl;
      return;
    }
  }
}

void VM::put_in(Object* o) {
  objects_.push_back(o);
}

void VM::gray_value(Value v) {
  if (v == nullptr)
    return;

  if (v->is_dark())
    return;
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << Xptr::address(v) << "` gray " << v << std::endl;
#endif
  v->set_dark(true);
  gray_stack_.push_back(v);
}

void VM::blacken_object(Object* obj) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << Xptr::address(obj) << "` blacken " << obj << std::endl;
#endif
  obj->blacken(*this);
}

void VM::free_object(Object* obj) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << Xptr::address(obj) << "` free " << obj << std::endl;
#endif
  delete obj;
}

void VM::interpret(const std::string& source_bytes) {
  Compile c;

  auto* fn = c.compile(*this, source_bytes);
  if (fn == nullptr)
    return;
  fn->dump();

  run(fn);
  collect();
  print_stack();
}

}
