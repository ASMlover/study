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

Value BaseObject::make_numeric(double d) {
  return new BaseObject(d);
}

std::ostream& operator<<(std::ostream& out, Value val) {
  return out << val->stringify();
}

struct CallFrame {
  int ip{};
  Block* block{};
  int locals{};

  CallFrame(int ip_arg, Block* block_arg, int locals_arg) noexcept
    : ip(ip_arg), block(block_arg), locals(locals_arg) {
  }
};

class Fiber : private UnCopyable {
  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;
public:
  inline void reset(void) {
    stack_.clear();
    frames_.clear();
  }

  inline CallFrame& peek_frame(void) {
    return frames_.back();
  }

  inline void pop_frame(void) {
    frames_.pop_back();
  }

  inline bool empty_frame(void) const {
    return frames_.empty();
  }

  inline Value peek_value(int distance = 0) const {
    return stack_[stack_.size() - 1 - distance];
  }

  inline Value get_value(int i) const {
    return stack_[i];
  }

  inline void set_value(int i, Value v) {
    if (stack_.size() <= i)
      stack_.push_back(v);
    else
      stack_[i] = v;
  }

  inline void push(Value v) {
    stack_.push_back(v);
  }

  inline Value pop(void) {
    Value v = stack_.back();
    stack_.pop_back();
    return v;
  }

  void call_block(Block* block, int locals) {
    frames_.push_back(CallFrame(0, block, locals));
  }
};

/// VM IMPLEMENTATIONS

void Class::append_method(VM& vm, const str_t& name, PrimitiveFn&& fn) {
  methods_[vm.ensure_symbol(name)] = std::move(fn);
}

VM::VM(void) {
  num_class_.append_method(*this, "abs", [](Value v) -> Value {
        auto d = v->as_numeric();
        if (d < 0)
          d = -d;
        return BaseObject::make_numeric(d);
      });
}

int VM::ensure_symbol(const str_t& name) {
  int existing = get_symbol(name);
  if (existing != -1)
    return existing;

  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

int VM::add_symbol(const str_t& name) {
  if (get_symbol(name) != -1)
    return -1;

  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

int VM::get_symbol(const str_t& name) const {
  for (sz_t i = 0; i < symbols_.size(); ++i) {
    if (symbols_[i] == name)
      return Xt::as_type<int>(i);
  }
  return -1;
}

void VM::clear_symbol(void) {
  symbols_.clear();
}

Value VM::interpret(Block* block) {
  Fiber fiber;
  fiber.reset();
  fiber.call_block(block, 0);

  for (;;) {
    auto* frame = &fiber.peek_frame();

    switch (auto c = Xt::as_type<Code>(frame->block->get_code(frame->ip++))) {
    case Code::CONSTANT:
      {
        Value v = frame->block->get_constant(frame->block->get_code(frame->ip++));
        fiber.push(v);
      } break;
    case Code::LOAD_LOCAL:
      {
        int local = frame->block->get_code(frame->ip++);
        fiber.push(fiber.get_value(frame->locals + local));
      } break;
    case Code::STORE_LOCAL:
      {
        int local = frame->block->get_code(frame->ip++);
        fiber.set_value(frame->locals + local, fiber.peek_value());
      } break;
    case Code::POP: fiber.pop(); break;
    case Code::CALL:
      {
        Value receiver = fiber.pop();
        int symbol = frame->block->get_code(frame->ip++);

        Class* cls = &num_class_;
        auto primitive_fn = cls->get_method(symbol);
        if (primitive_fn) {
          Value r = primitive_fn(receiver);
          fiber.push(r);
        }
        else {
          std::cerr << "not method" << std::endl;
          std::exit(-1);
        }
      } break;
    case Code::END:
      {
        Value r = fiber.pop();
        fiber.pop_frame();

        if (fiber.empty_frame())
          return r;
        fiber.set_value(frame->locals, r);
      } break;
    }
  }
  return nullptr;
}

void VM::interpret(const str_t& source_bytes) {
  Block* block = compile(*this, source_bytes);

  Value r = interpret(block);
  std::cout << r << std::endl;
}

}
