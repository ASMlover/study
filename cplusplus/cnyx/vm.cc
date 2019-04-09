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
#include <cstdarg>
#include <iostream>
#include <sstream>
#include "compile.hh"
#include "vm.hh"

namespace nyx {

class CallFrame : private UnCopyable {
  using IterCallback = std::function<void(FunctionObject*, const u8_t*)>;

  FunctionObject* fn_{};
  u8_t* ip_{};
  int stack_start_{};
public:
  CallFrame(FunctionObject* fn, u8_t* ip, int start)
    : fn_(fn), ip_(ip), stack_start_(start) {
  }

  CallFrame(CallFrame&& frame)
    : fn_(std::move(frame.fn_))
    , ip_(std::move(frame.ip_))
    , stack_start_(std::move(frame.stack_start_)) {
  }

  inline FunctionObject* fn(void) const { return fn_; }
  inline u8_t* ip(void) const { return ip_; }
  inline void set_ip(u8_t* ip) { ip_ = ip; }
  inline u8_t get_ip(int i) { return ip_[i]; }
  inline u8_t inc_ip(void) { return *ip_++; }
  inline u8_t dec_ip(void) { return *ip_--; }
  inline void add_ip(int offset) { ip_ += offset; }
  inline void sub_ip(int offset) { ip_ -= offset; }
  inline int stack_start(void) const { return stack_start_; }
  inline const u8_t* get_fn_codes(void) const { return fn_->codes(); }
  inline Value get_fn_constant(int i) const { return fn_->get_constant(i); }

  inline void visit(IterCallback&& cb) { cb(fn_, ip_); }
};

VM::VM(void) {
  globals_ = TableObject::create(*this);
  globals_->set_entry(
      StringObject::create(*this, "print", 5),
      NativeObject::create(*this, [](int argc, Value* args) -> Value {
          for (int i = 0; i < argc; ++i)
            std::cout << args[i] << " ";
          std::cout << std::endl;
          return nullptr;
        }));
}

VM::~VM(void) {
  globals_ = nullptr;

  for (auto* o : objects_)
    free_object(o);
  objects_.clear();
  gray_stack_.clear();
}

void VM::push(Value val) {
  stack_.push_back(val);
}

Value VM::pop(void) {
  Value val = stack_.back();
  stack_.pop_back();
  return val;
}

Value VM::peek(int distance) const {
  return stack_[stack_.size() - 1 - distance];
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  std::cerr << std::endl;

  for (auto i = frames_.size() - 1; i >= 0; --i) {
    auto& frame = frames_[i];
    frame.visit([](FunctionObject* fn, const u8_t* ip) {
          auto i = static_cast<int>(ip - fn->codes());
          std::cerr << "[LINE: " << fn->get_codeline(i) << "]" << std::endl;
        });
  }
}

std::optional<bool> VM::pop_boolean(void) {
  if (!BaseObject::is_boolean(peek())) {
    runtime_error("operand must be a boolean");
    return {};
  }
  return {Xptr::down<BooleanObject>(pop())->value()};
}

std::optional<double> VM::pop_numeric(void) {
  if (!BaseObject::is_numeric(peek())) {
    runtime_error("operand must be a numeric");
    return {};
  }
  return {Xptr::down<NumericObject>(pop())->value()};
}

std::optional<std::tuple<double, double>> VM::pop_numerics(void) {
  if (!BaseObject::is_numeric(peek(0))) {
    runtime_error("right operand must be a numeric");
    return {};
  }
  if (!BaseObject::is_numeric(peek(1))) {
    runtime_error("left operand must be a numeric");
    return {};
  }

  auto b = Xptr::down<NumericObject>(pop())->value();
  auto a = Xptr::down<NumericObject>(pop())->value();
  return {{a, b}};
}

void VM::collect(void) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "********* collect: starting *********" << std::endl;
#endif

  // mark the stack roots
  for (auto* o : stack_)
    gray_value(o);
  for (auto& f : frames_)
    gray_value(f.fn());

  gray_value(globals_);
  gray_compiler_roots(*this);

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

bool VM::call(FunctionObject* fn, int argc/* = 0*/) {
  if (argc < fn->arity()) {
    runtime_error("not enough arguments");
    return false;
  }

  frames_.emplace_back(CallFrame(
        fn, fn->codes(), static_cast<int>(stack_.size() - fn->arity())));
  return true;
}

bool VM::run(void) {
  auto* frame = &frames_.back();

  auto _rdbyte = [&frame](void) -> u8_t { return frame->inc_ip(); };
  auto _rdword = [&frame](void) -> u16_t {
    return (frame->add_ip(2),
        static_cast<u16_t>((frame->get_ip(-2) << 8) | frame->get_ip(-1)));
  };

  for (;;) {
#if defined(DEBUG_EXEC_TRACE)
    for (auto* o : stack_)
      std::cout << "| " << o << " ";
    std::cout << std::endl;
    frame->fn()->dump_instruction(
        static_cast<int>(frame->ip() - frame->get_fn_codes()));
#endif

    switch (auto instruction = frame->inc_ip(); instruction) {
    case OpCode::OP_CONSTANT:
      {
        u8_t constant = _rdbyte();
        push(frame->get_fn_constant(constant));
      } break;
    case OpCode::OP_NIL: push(nullptr); break;
    case OpCode::OP_POP: pop(); break;
    case OpCode::OP_GET_LOCAL:
      {
        u8_t slot = _rdbyte();
        push(stack_[frame->stack_start() + slot]);
      } break;
    case OpCode::OP_SET_LOCAL:
      {
        u8_t slot = _rdbyte();
        stack_[frame->stack_start() + slot] = peek();
      } break;
    case OpCode::OP_DEF_GLOBAL:
      {
        u8_t constant = _rdbyte();
        auto* key = Xptr::down<StringObject>(frame->get_fn_constant(constant));
        globals_->set_entry(key, pop());
      } break;
    case OpCode::OP_GET_GLOBAL:
      {
        u8_t constant = _rdbyte();
        auto* key = Xptr::down<StringObject>(frame->get_fn_constant(constant));
        if (auto val = globals_->get_entry(key); val) {
          push(*val);
        }
        else {
          runtime_error("undefined variable `%s`", key->chars());
          return false;
        }
      } break;
    case OpCode::OP_SET_GLOBAL:
      {
        u8_t constant = _rdbyte();
        auto* key = Xptr::down<StringObject>(frame->get_fn_constant(constant));
        if (!globals_->set_entry(key, peek())) {
          runtime_error("undefined variable `%s`", key->chars());
          return false;
        }
      } break;
    case OpCode::OP_EQ:
      {
        auto b = pop();
        auto a = pop();
        push(BooleanObject::create(*this, values_equal(a, b)));
      } break;
    case OpCode::OP_NE:
      {
        auto b = pop();
        auto a = pop();
        push(BooleanObject::create(*this, !values_equal(a, b)));
      } break;
    case OpCode::OP_GT:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(BooleanObject::create(*this, a > b));
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_GE:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(BooleanObject::create(*this, a >= b));
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_LT:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(BooleanObject::create(*this, a < b));
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_LE:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(BooleanObject::create(*this, a <= b));
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_ADD:
      {
        if (BaseObject::is_string(peek(0)) && BaseObject::is_string(peek(1))) {
          auto* b = Xptr::down<StringObject>(pop());
          auto* a = Xptr::down<StringObject>(pop());
          push(StringObject::concat(*this, a, b));
        }
        else if (BaseObject::is_numeric(peek(1))
            && BaseObject::is_numeric(peek(1))) {
          double b = Xptr::down<NumericObject>(pop())->value();
          double a = Xptr::down<NumericObject>(pop())->value();
          push(NumericObject::create(*this, a + b));
        }
        else {
          runtime_error("can only add two strings or two numerics");
          return false;
        }
      } break;
    case OpCode::OP_SUB:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(NumericObject::create(*this, a - b));
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_MUL:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(NumericObject::create(*this, a * b));
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_DIV:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(NumericObject::create(*this, a / b));
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_NOT:
      {
        if (auto b = pop_boolean(); b)
          push(BooleanObject::create(*this, !*b));
        else
          return false;
      } break;
    case OpCode::OP_NEG:
      {
        if (auto v = pop_numeric(); v)
          push(NumericObject::create(*this, -*v));
        else
          return false;
      } break;
    case OpCode::OP_JUMP:
      {
        u16_t offset = _rdword();
        frame->add_ip(offset);
      } break;
    case OpCode::OP_JUMP_IF_FALSE:
      {
        u16_t offset = _rdword();
        Value cond = peek();
        if (BaseObject::is_nil(cond) || (cond->type() == ObjType::BOOLEAN &&
              !Xptr::down<BooleanObject>(cond)->value())) {
          frame->add_ip(offset);
        }
      } break;
    case OpCode::OP_LOOP:
      {
        u16_t offset = _rdword();
        frame->sub_ip(offset);
      } break;
    case OpCode::OP_CALL_0:
    case OpCode::OP_CALL_1:
    case OpCode::OP_CALL_2:
    case OpCode::OP_CALL_3:
    case OpCode::OP_CALL_4:
    case OpCode::OP_CALL_5:
    case OpCode::OP_CALL_6:
    case OpCode::OP_CALL_7:
    case OpCode::OP_CALL_8:
      {
        int argc = instruction - OpCode::OP_CALL_0;
        Value called = peek(argc);
        if (BaseObject::is_native(called)) {
          Value ret = Xptr::down<NativeObject>(called)->get_function()(
              argc, &stack_[stack_.size() - argc]);
          stack_.resize(stack_.size() - argc - 1);
          push(ret);
        }
        else if (BaseObject::is_function(called)) {
          auto* func = Xptr::down<FunctionObject>(called);
          if (!call(func, argc))
            return false;
          frame = &frames_.back();
        }
        else {
          runtime_error("can only call functions and classes");
          return false;
        }
      } break;
    case OpCode::OP_RETURN:
      {
        Value ret = pop();
        if (frames_.size() == 1)
          return true;

        stack_.resize(frame->stack_start() - 1);
        push(ret);

        frames_.pop_back();
        frame = &frames_.back();
      } break;
    }
  }

  return true;
}

void VM::gray_value(Value v) {
  if (BaseObject::is_nil(v))
    return;

  if (v->is_dark())
    return;
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << Xptr::address(v) << "` gray " << v << std::endl;
#endif
  v->set_dark(true);
  gray_stack_.push_back(v);
}

void VM::blacken_object(BaseObject* obj) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << Xptr::address(obj) << "` blacken " << obj << std::endl;
#endif
  obj->blacken(*this);
}

void VM::free_object(BaseObject* obj) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << Xptr::address(obj) << "` free " << obj << std::endl;
#endif
  delete obj;
}

InterpretResult VM::interpret(const str_t& source_bytes) {
  Compile c;

  stack_.clear();
  frames_.clear();

  auto* fn = c.compile(*this, source_bytes);
  if (fn == nullptr)
    return InterpretResult::COMPILE_ERROR;
  // fn->dump();
  call(fn, 0);

  // collect();
  // print_stack();

  return run() ? InterpretResult::OK : InterpretResult::RUNTIME_ERROR;
}

}
