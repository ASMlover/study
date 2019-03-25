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
#include <sstream>
#include "vm.hh"

namespace nyx {

std::size_t Froward::size(void) const {
  return sizeof(*this);
}

std::string Froward::stringify(void) const {
  std::stringstream ss;
  ss << "fwd->" << reinterpret_cast<void*>(to_);
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
  ss << "(" << first_->stringify() << ", " << second_->stringify() << ")";
  return ss.str();
}

VM::VM(void) {
  initialize();
}

void VM::initialize(void) {
  from_beg_ = new byte_t[kMaxHeap];
  from_end_ = from_beg_;
  to_beg_ = new byte_t[kMaxHeap];
  to_end_ = to_beg_;
}

void VM::collect(void) {
  // TODO:
}

void VM::push(double value) {
  // TODO:
}

void VM::push(void) {
  // TODO:
}

void VM::pop(void) {
  // TODO:
}

void VM::print_stack(void) {
  int i{};
  for (auto* o : stack_)
    std::cout << i++ << ":" << o->stringify() << std::endl;
}

}
