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
#include "compiler.hh"
#include "vm.hh"

namespace wrencc {

BaseObject::BaseObject(double d)
  : type_(ObjType::NUMERIC) {
  as_.numeric = d;
}

str_t BaseObject::stringify(void) const {
  auto _double2str_fn = [](double d) -> str_t {
    std::stringstream ss;
    ss << d;
    return ss.str();
  };

  switch (type_) {
  case ObjType::NUMERIC: return _double2str_fn(as_.numeric);
  }
  return "";
}

std::ostream& operator<<(std::ostream& out, Value val) {
  return out << val->stringify();
}

class Fiber : private UnCopyable {
  std::vector<Value> stack_;
public:
  inline void push(Value v) {
    stack_.push_back(v);
  }

  inline Value pop(void) {
    Value v = stack_.back();
    stack_.pop_back();
    return v;
  }
};

/// VM IMPLEMENTATIONS

int VM::get_symbol(const str_t& name) {
  for (sz_t i = 0; i < symbols_.size(); ++i) {
    if (symbols_[i] == name)
      return Xt::as_type<int>(i);
  }

  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

Value VM::interpret(Block* block) {
  Fiber fiber;

  int ip{};
  for (;;) {
    switch (auto c = Xt::as_type<Code>(block->get_code(ip++))) {
    case Code::CONSTANT:
      {
        Value v = block->get_constant(block->get_code(ip++));
        fiber.push(v);
      } break;
    case Code::CALL:
      {
        int symbol = block->get_code(ip++);
        std::cout << "call " << symbol << std::endl;
      } break;
    case Code::END: return fiber.pop();
    }
  }
}

void VM::interpret(const str_t& source_bytes) {
  Block* block = compile(*this, source_bytes);

  Value r = interpret(block);
  std::cout << r << std::endl;
}

}
