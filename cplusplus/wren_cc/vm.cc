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

std::ostream& operator<<(std::ostream& out, Value val) {
  return out << val->stringify();
}

str_t NumericObject::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

NumericObject* NumericObject::make_numeric(double d) {
  return new NumericObject(d);
}

str_t BlockObject::stringify(void) const {
  return "[block]";
}

BlockObject* BlockObject::make_block(void) {
  return new BlockObject();
}

ClassObject::ClassObject(void) noexcept
  : BaseObject(ObjType::CLASS) {
}

str_t ClassObject::stringify(void) const {
  return "[class]";
}

ClassObject* ClassObject::make_class(void) {
  return new ClassObject();
}

int SymbolTable::ensure(const str_t& name) {
  int existing = get(name);
  if (existing != -1)
    return existing;

  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

int SymbolTable::add(const str_t& name) {
  if (get(name) != -1)
    return -1;

  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

int SymbolTable::get(const str_t& name) const {
  for (sz_t i = 0; i < symbols_.size(); ++i) {
    if (symbols_[i] == name)
      return Xt::as_type<int>(i);
  }
  return -1;
}

void SymbolTable::clear(void) {
  symbols_.clear();
}

struct CallFrame {
  int ip{};
  BlockObject* block{};
  int locals{};

  CallFrame(int ip_arg, BlockObject* block_arg, int locals_arg) noexcept
    : ip(ip_arg), block(block_arg), locals(locals_arg) {
  }

  inline u8_t get_code(int i) const { return block->get_code(i); }
  inline Value get_constant(int i) const { return block->get_constant(i); }
};

class Fiber : private UnCopyable {
  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;
public:
  inline void reset(void) {
    stack_.clear();
    frames_.clear();
  }

  inline int stack_size(void) const { return Xt::as_type<int>(stack_.size()); }
  inline int frame_size(void) const { return Xt::as_type<int>(frames_.size()); }

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

  void call_block(BlockObject* block, int beglocal = 0) {
    frames_.push_back(CallFrame(0, block, beglocal));

    for (int i = 0; i < block->num_locals(); ++i)
      push(nullptr);
  }
};

/// VM IMPLEMENTATIONS

static Value _primitive_numabs(Value v) {
  double d = Xt::down<NumericObject>(v)->value();
  if (d < 0)
    d = -d;
  return NumericObject::make_numeric(d);
}

VM::VM(void) {
  num_class_ = ClassObject::make_class();
  int symbol = symbols_.ensure("abs");
  num_class_->set_method(symbol, MethodType::PRIMITIVE, _primitive_numabs);
}

Value VM::interpret(BlockObject* block) {
  Fiber fiber;
  fiber.reset();
  fiber.call_block(block, 0);

  for (;;) {
    auto* frame = &fiber.peek_frame();

    switch (auto c = Xt::as_type<Code>(frame->get_code(frame->ip++))) {
    case Code::CONSTANT:
      {
        Value v = frame->get_constant(frame->get_code(frame->ip++));
        fiber.push(v);

        std::cout
          << "load constant `" << v
          << "` to `" << fiber.stack_size() - 1 << "`" << std::endl;
      } break;
    case Code::CLASS:
      {
        fiber.push(ClassObject::make_class());
        std::cout << "push class at: " << fiber.stack_size() - 1 << std::endl;
      } break;
    case Code::LOAD_LOCAL:
      {
        int local = frame->get_code(frame->ip++);
        fiber.push(fiber.get_value(frame->locals + local));

        std::cout
          << "load local `" << local << "` "
          << "to `" << fiber.stack_size() - 1 << "`" << std::endl;
      } break;
    case Code::STORE_LOCAL:
      {
        int local = frame->get_code(frame->ip++);
        std::cout
          << "store local `" << local << "` "
          << "from `" << fiber.stack_size() - 1 << "`" << std::endl;
        fiber.set_value(frame->locals + local, fiber.peek_value());
      } break;
    case Code::DUP:
      {
        fiber.push(fiber.peek_value());
        std::cout << "dup `" << fiber.stack_size() - 1 << "`" << std::endl;
      } break;
    case Code::POP:
      std::cout << "pop `" << fiber.stack_size() - 1 << "`" << std::endl;
      fiber.pop(); break;
    case Code::CALL:
      {
        Value receiver = fiber.pop();
        int symbol = frame->get_code(frame->ip++);

        ClassObject* cls = num_class_;
        auto& method = cls->get_method(symbol);
        switch (method.type) {
        case MethodType::NONE:
          std::cerr << "no method" << std::endl;
          std::exit(-1); break;
        case MethodType::PRIMITIVE:
          fiber.push(method.primitive(receiver)); break;
        case MethodType::BLOCK:
          fiber.call_block(method.block, fiber.stack_size()); break;
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
  auto* block = compile(*this, source_bytes);

  Value r = interpret(block);
  std::cout << r << std::endl;
}

}
