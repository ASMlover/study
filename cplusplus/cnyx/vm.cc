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
  initialize();
}

VM::~VM(void) {
  delete [] heaptr_;
}

void VM::initialize(void) {
  heaptr_ = new byte_t[kMaxHeap << 1];
  fromspace_ = heaptr_;
  tospace_ = heaptr_ + kMaxHeap;
  allocptr_ = fromspace_;
}

void VM::collect(void) {
  std::cout << "********* collect: starting *********" << std::endl;

  std::swap(fromspace_, tospace_);
  allocptr_ = fromspace_;

  for (auto i = 0u; i < stack_.size(); ++i)
    stack_[i] = move_object(stack_[i]);
  trace_compiler_roots();

  byte_t* p = fromspace_;
  while (p < allocptr_) {
    auto* obj = as_object(p);
    obj->traverse(*this);
    p += obj->size();
  }

  std::cout << "********* collect: finished *********" << std::endl;
}

void VM::print_stack(void) {
  int i{};
  for (auto* o : stack_)
    std::cout << i++ << " : " << o << std::endl;
}

void VM::run(FunctionObject* fn) {
  const std::uint8_t* ip = fn->raw_codes();
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

Value VM::move_object(Value from_ref) {
  if (from_ref == nullptr)
    return nullptr;
  if (from_ref->get_type() == ObjType::FORWARD)
    return from_ref->down_to<ForwardObject>()->to();

  auto* p = allocptr_;
  allocptr_ += from_ref->size();

  std::cout
    << "copy " << from_ref << " from `" << from_ref->address()
    << "` to `" << as_address(p) << "`" << std::endl;

  Object* to_ref = from_ref->move_to(p);
  auto* old = ForwardObject::forward(from_ref->address());
  old->set_to(to_ref);

  return to_ref;
}

void* VM::allocate(std::size_t n) {
  if (allocptr_ + n > fromspace_ + kMaxHeap) {
    collect();

    if (allocptr_ + n > fromspace_ + kMaxHeap) {
      std::cerr
        << "Heap full, need " << n
        << " bytes, but only " << (kMaxHeap - (allocptr_ - fromspace_))
        << " available" << std::endl;
      std::exit(-1);
    }
  }
  else {
#if defined(DEBUG_GC_STRESS)
    collect();
#endif
  }

  auto* r = allocptr_;
  allocptr_ += n;
  return r;
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
