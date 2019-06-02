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
#include "primitives.hh"
#include "compiler.hh"
#include "vm.hh"

namespace wrencc {

std::ostream& operator<<(std::ostream& out, Value val) {
  return out << val->stringify();
}

NumericObject* BaseObject::as_numeric(void) const {
  return Xt::down<NumericObject>(const_cast<BaseObject*>(this));
}

StringObject* BaseObject::as_string(void) const {
  return Xt::down<StringObject>(const_cast<BaseObject*>(this));
}

BlockObject* BaseObject::as_block(void) const {
  return Xt::down<BlockObject>(const_cast<BaseObject*>(this));
}

ClassObject* BaseObject::as_class(void) const {
  return Xt::down<ClassObject>(const_cast<BaseObject*>(this));
}

InstanceObject* BaseObject::as_instance(void) const {
  return Xt::down<InstanceObject>(const_cast<BaseObject*>(this));
}

str_t NumericObject::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

NumericObject* NumericObject::make_numeric(double d) {
  return new NumericObject(d);
}

str_t StringObject::stringify(void) const {
  return value_;
}

StringObject* StringObject::make_string(const char* s) {
  return new StringObject(s);
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

ClassObject::ClassObject(ClassObject* meta_class) noexcept
  : BaseObject(ObjType::CLASS)
  , meta_class_(meta_class) {
}

str_t ClassObject::stringify(void) const {
  return "[class]";
}

ClassObject* ClassObject::make_class(void) {
  auto* meta_class = new ClassObject();
  return new ClassObject(meta_class);
}

InstanceObject::InstanceObject(ClassObject* cls) noexcept
  : BaseObject(ObjType::INSTANCE)
  , cls_(cls) {
}

str_t InstanceObject::stringify(void) const {
  return "[instance]";
}

InstanceObject* InstanceObject::make_instance(ClassObject* cls) {
  return new InstanceObject(cls);
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

  inline Value* values_at(int i) { return &stack_[i]; }
  inline void resize_stack(int n) { stack_.resize(n); }
  inline int stack_size(void) const { return Xt::as_type<int>(stack_.size()); }
  inline int frame_size(void) const { return Xt::as_type<int>(frames_.size()); }
  inline CallFrame& peek_frame(void) { return frames_.back(); }
  inline void pop_frame(void) { frames_.pop_back(); }
  inline bool empty_frame(void) const { return frames_.empty(); }
  inline Value get_value(int i) const { return stack_[i]; }
  inline void set_value(int i, Value v) { stack_[i] = v; }

  inline Value peek_value(int distance = 0) const {
    return stack_[stack_.size() - 1 - distance];
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

static Value _primitive_metaclass_new(int argc, Value* args) {
  ClassObject* cls = args[0]->as_class();
  return InstanceObject::make_instance(cls);
}

VM::VM(void) {
  block_class_ = ClassObject::make_class();
  class_class_ = ClassObject::make_class();
  num_class_ = ClassObject::make_class();
  str_class_ = ClassObject::make_class();

  // the call method is special:
  {
    int symbol = symbols_.ensure("call");
    block_class_->set_method(symbol, MethodType::CALL);
  }

  reigister_primitives(*this);
}

void VM::set_primitive(ClassObject* cls, const str_t& name, PrimitiveFn fn) {
  int symbol = symbols_.ensure(name);
  cls->set_method(symbol, MethodType::PRIMITIVE, fn);
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
        ClassObject* cls = ClassObject::make_class();

        // define `new` method on the metaclass.
        int new_symbol = symbols_.ensure("new");
        std::cout << "define `new` " << new_symbol << std::endl;

        cls->meta_class()->set_method(
            new_symbol, MethodType::PRIMITIVE, _primitive_metaclass_new);
        fiber.push(cls);

        std::cout << "push class at: " << fiber.stack_size() - 1 << std::endl;
      } break;
    case Code::METHOD:
      {
        int symbol = frame->get_code(frame->ip++);
        int constant = frame->get_code(frame->ip++);
        ClassObject* cls = fiber.peek_value()->as_class();

        BlockObject* body = frame->get_constant(constant)->as_block();
        cls->set_method(symbol, MethodType::BLOCK, body);

        std::cout
          << "define method `" << symbol
          << "` using constant `" << constant << "` on " << cls << std::endl;
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
    case Code::LOAD_GLOBAL:
      {
        int global = frame->get_code(frame->ip++);
        fiber.push(globals_[global]);
        std::cout
          << "load global `" << global << "` "
          << "to `" << fiber.stack_size() - 1 << "`" << std::endl;
      } break;
    case Code::STORE_GLOBAL:
      {
        int global = frame->get_code(frame->ip++);
        std::cout
          << "store global `" << global << "` "
          << "from `" << fiber.stack_size() - 1 << "`" << std::endl;
        globals_[global] = fiber.get_value(fiber.stack_size() - 1);
      } break;
    case Code::DUP:
      {
        fiber.push(fiber.peek_value());
        std::cout << "dup `" << fiber.stack_size() - 1 << "`" << std::endl;
      } break;
    case Code::POP:
      std::cout << "pop `" << fiber.stack_size() - 1 << "`" << std::endl;
      fiber.pop(); break;
    case Code::CALL_0:
    case Code::CALL_1:
    case Code::CALL_2:
    case Code::CALL_3:
    case Code::CALL_4:
    case Code::CALL_5:
    case Code::CALL_6:
    case Code::CALL_7:
    case Code::CALL_8:
    case Code::CALL_9:
    case Code::CALL_10:
      {
        int argc = c - Code::CALL_0;
        Value receiver = fiber.get_value(fiber.stack_size() - argc - 1);

        int symbol = frame->get_code(frame->ip++);

        ClassObject* cls{};
        switch (receiver->type()) {
        case ObjType::BLOCK: cls = block_class_; break;
        case ObjType::CLASS: cls = receiver->as_class()->meta_class(); break;
        case ObjType::NUMERIC: cls = num_class_; break;
        case ObjType::STRING: cls = str_class_; break;
        case ObjType::INSTANCE: cls = receiver->as_instance()->cls(); break;
        }

        std::cout << "call `" << symbol << "` on " << receiver << std::endl;

        auto& method = cls->get_method(symbol);
        switch (method.type) {
        case MethodType::NONE:
          std::cerr << "no method" << std::endl;
          std::exit(-1); break;
        case MethodType::CALL:
          fiber.call_block(receiver->as_block(), fiber.stack_size() - argc);
          break;
        case MethodType::PRIMITIVE:
          fiber.set_value(fiber.stack_size() - argc - 1,
              method.primitive(argc,
                fiber.values_at(fiber.stack_size() - argc - 1)));
          fiber.resize_stack(fiber.stack_size() - argc);
          break;
        case MethodType::BLOCK:
          fiber.call_block(method.block, fiber.stack_size() - argc);
          break;
        }
      } break;
    case Code::END:
      {
        Value r = fiber.pop();
        fiber.pop_frame();

        if (fiber.empty_frame()) {
          std::cout << "done with result `" << r << "`" << std::endl;
          return r;
        }

        std::cout
          << "return and store result `" << r
          << "` in " << frame->locals << std::endl;
        if (fiber.stack_size() <= frame->locals)
          fiber.push(r);
        else
          fiber.set_value(frame->locals, r);

        fiber.resize_stack(frame->locals + 1);
      } break;
    }
  }
  return nullptr;
}

void VM::interpret(const str_t& source_bytes) {
  auto* block = compile(*this, source_bytes);

  if (block != nullptr) {
    Value r = interpret(block);
    std::cout << "run VM interpret result is: `" << r << "`" << std::endl;
  }
}

}
