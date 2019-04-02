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
}

VM::~VM(void) {
  for (auto* o : objects_)
    free_object(o);
  objects_.clear();
  gray_stack_.clear();
}

void VM::collect(void) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "********* collect: starting *********" << std::endl;
#endif

  // mark the roots
  for (auto* v : stack_)
    gray_value(v);

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
  const std::uint8_t* ip = fn->codes();
  for (;;) {
    switch (*ip++) {
    case OpCode::OP_CONSTANT:
      {
        std::uint8_t constant = *ip++;
        push(fn->get_constant(constant));
      } break;
    case OpCode::OP_ADD:
      {
        // TODO: check types
        double b = pop()->down_to<NumericObject>()->value();
        double a = pop()->down_to<NumericObject>()->value();
        push(NumericObject::create(*this, a + b));
      } break;
    case OpCode::OP_SUB:
      {
        // TODO: check types
        double b = pop()->cast_to<NumericObject>()->value();
        double a = pop()->cast_to<NumericObject>()->value();
        push(NumericObject::create(*this, a - b));
      } break;
    case OpCode::OP_MUL:
      {
        // TODO: check types
        double b = pop()->cast_to<NumericObject>()->value();
        double a = pop()->cast_to<NumericObject>()->value();
        push(NumericObject::create(*this, a * b));
      } break;
    case OpCode::OP_DIV:
      {
        // TODO: check types
        double b = pop()->cast_to<NumericObject>()->value();
        double a = pop()->cast_to<NumericObject>()->value();
        push(NumericObject::create(*this, a / b));
      } break;
    case OpCode::OP_RETURN:
      // std::cout << stack_.back() << std::endl;
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
  std::cout << "`" << v->address() << "` gray " << v << std::endl;
#endif
  v->set_dark(true);
  gray_stack_.push_back(v);
}

void VM::blacken_object(Object* obj) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << obj->address() << "` blacken " << obj << std::endl;
#endif
  obj->blacken(*this);
}

void VM::free_object(Object* obj) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << obj->address() << "` free " << obj << std::endl;
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
