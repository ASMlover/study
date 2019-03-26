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
#include "vm.hh"

namespace nyx {

std::ostream& operator<<(std::ostream& out, Object* o) {
  return out << o->stringify();
}

std::size_t Forward::size(void) const {
  return sizeof(*this);
}

std::string Forward::stringify(void) const {
  std::stringstream ss;
  ss << "fwd->" << to_->address();
  return ss.str();
}

std::size_t Numeric::size(void) const {
  return sizeof(*this);
}

std::string Numeric::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

std::size_t Pair::size(void) const {
  return sizeof(*this);
}

std::string Pair::stringify(void) const {
  std::stringstream ss;
  ss << "(" << first_ << ", " << second_ << ")";
  return ss.str();
}

VM::VM(void) {
  initialize();
}

VM::~VM(void) {
  delete [] from_beg_;
  delete [] to_beg_;
}

void VM::initialize(void) {
  from_beg_ = new byte_t[kMaxHeap];
  from_end_ = from_beg_;
  to_beg_ = new byte_t[kMaxHeap];
  to_end_ = to_beg_;
}

Value VM::copy(Value from_ref) {
  if (from_ref->get_type() == ObjType::FORWARD)
    return from_ref->cast_to<Forward*>()->to();

  std::cout
    << "copy " << from_ref << " from " << from_ref->address()
    << " to " << reinterpret_cast<void*>(to_end_) << std::endl;

  auto size = from_ref->size();
  Object* to_ref{};
  switch (from_ref->get_type()) {
  case ObjType::NUMERIC:
    to_ref = new (to_end_) Numeric(std::move(*from_ref->cast_to<Numeric*>()));
    break;
  case ObjType::FORWARD:
    to_ref = new (to_end_) Forward(std::move(*from_ref->cast_to<Forward*>()));
    break;
  case ObjType::PAIR:
    to_ref = new (to_end_) Pair(std::move(*from_ref->cast_to<Pair*>()));
    break;
  }

  auto* old = new (from_ref->address()) Forward();
  old->set_to(to_ref);
  to_end_ += size;

  return to_ref;
}

void VM::copy_references(Object* obj) {
  switch (obj->get_type()) {
  case ObjType::FORWARD: assert(false); break;
  case ObjType::NUMERIC: break;
  case ObjType::PAIR:
    {
      auto* pair = obj->cast_to<Pair*>();
      pair->set_first(copy(pair->first()));
      pair->set_second(copy(pair->second()));
    } break;
  }
}

void VM::collect(void) {
  for (auto i = 0u; i < stack_.size(); ++i)
    stack_[i] = copy(stack_[i]);

  byte_t* ptr = to_beg_;
  while (ptr < to_end_) {
    auto* obj = as_object(ptr);
    copy_references(obj);
    ptr += obj->size();
  }

  byte_t* tmp = from_beg_;
  from_beg_ = to_beg_;
  from_end_ = to_end_;
  to_beg_ = to_end_ = tmp;
}

void* VM::allocate(std::size_t n) {
  if (from_end_ + n > from_beg_ + kMaxHeap) {
    collect();

    if (from_end_ + n > from_beg_ + kMaxHeap) {
      std::cerr
        << "Heap full, need " << n
        << " bytes, but only " << (kMaxHeap - (from_end_ - from_beg_))
        << " available" << std::endl;
      std::exit(-1);
    }
  }

  auto* r = from_end_;
  from_end_ += n;
  return r;
}

void VM::push_numeric(double value) {
  auto* obj = new (allocate(sizeof(Numeric))) Numeric();
  obj->set_value(value);
  stack_.push_back(obj);
}

void VM::push_pair(void) {
  auto* obj = new (allocate(sizeof(Pair))) Pair();
  obj->set_second(pop());
  obj->set_first(pop());
  stack_.push_back(obj);
}

Value VM::pop(void) {
  Value r = stack_.back();
  stack_.pop_back();
  return r;
}

void VM::print_stack(void) {
  int i{};
  for (auto* o : stack_)
    std::cout << i++ << ":" << o << std::endl;
}

}
