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
  using IterCallback =
    std::function<void(FunctionObject*, const u8_t*, std::vector<Value>&)>;

  FunctionObject* fn_{};
  const u8_t* ip_{};

  std::vector<Value> stack_;
  CallFrame* caller_{};

  CallFrame(void) {}
  CallFrame(FunctionObject* f, const u8_t* i, CallFrame* c = nullptr)
    : fn_(f), ip_(i), caller_(c) {
  }
public:
  inline FunctionObject* fn(void) const { return fn_; }
  inline const u8_t* ip(void) const { return ip_; }
  inline int stack_size(void) const { return static_cast<int>(stack_.size()); }
  inline Value* stack_values(int offset = 0) { return &stack_[offset]; }
  inline void resize_stack(int capacity) { stack_.resize(capacity); }
  inline const std::vector<Value>& get_stack(void) const { return stack_; }
  inline void set_stack_value(int i, Value v) { stack_[i] = v; }
  inline Value get_stack_value(int i) const { return stack_[i]; }
  inline Value get_fun_constant(int i) const { return fn_->get_constant(i); }

  void append_to_stack(Value v) {
    stack_.push_back(v);
  }

  Value pop_from_stack(void) {
    Value r = stack_.back();
    stack_.pop_back();
    return r;
  }

  Value stack_peek(int distance = 0) const {
    return stack_[stack_.size() - 1 - distance];
  }

  void iter_frames(IterCallback&& cb) {
    for (auto* f = this; f != nullptr; f = f->caller_)
      cb(f->fn_, f->ip_, f->stack_);
  }

  static CallFrame* create(void) {
    return new CallFrame();
  }

  static CallFrame* create(FunctionObject* fn, const u8_t* ip, CallFrame* c) {
    return new CallFrame(fn, ip, c);
  }

  static void release(CallFrame* frame) {
    for (auto* c = frame->caller_; c != nullptr; c = c->caller_)
      delete c;
    delete frame;
  }
};

VM::VM(void) {
  globals_ = TableObject::create(*this);

  auto* frame = CallFrame::create();
  frame->append_to_stack(StringObject::create(*this, "print", 5));
  frame->append_to_stack(NativeObject::create(*this,
        [](int argc, Value* args) -> Value {
          for (int i = 0; i < argc; ++i)
            std::cout << args[i] << " ";
          std::cout << std::endl;
          return nullptr;
        }));
  globals_->set_entry(Xptr::down<StringObject>(
        frame->get_stack_value(0)), frame->get_stack_value(1));
}

VM::~VM(void) {
  globals_ = nullptr;
  if (frame_ != nullptr)
    delete frame_;

  for (auto* o : objects_)
    free_object(o);
  objects_.clear();
  gray_stack_.clear();
}

void VM::push(Value val) {
  frame_->append_to_stack(val);
}

Value VM::pop(void) {
  return frame_->pop_from_stack();
}

Value VM::peek(int distance) const {
  return frame_->stack_peek(distance);
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  std::cerr << std::endl;
  va_end(ap);

  if (frame_ != nullptr) {
    frame_->iter_frames(
        [](FunctionObject* fn, const u8_t* ip, std::vector<Value>&) {
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
  if (frame_ != nullptr) {
    frame_->iter_frames(
        [this](FunctionObject* fn, const u8_t* ip, std::vector<Value>& s) {
          gray_value(fn);
          for (auto* o : s)
            gray_value(o);
        });
  }

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
  for (auto* o : frame_->get_stack())
    std::cout << i++ << " : " << o << std::endl;
}

void VM::call(FunctionObject* fn) {
  auto* frame = CallFrame::create(fn, fn->codes(), frame_);
  frame_ = frame;
}

bool VM::run(void) {
  const u8_t* ip = frame_->ip();

  auto _rdbyte = [&ip](void) -> u8_t { return *ip++; };
  auto _rdword = [&ip](void) -> u16_t {
    return (ip += 2, static_cast<u16_t>((ip[-2] << 8) | ip[-1]));
  };

  for (;;) {
#if defined(DEBUG_EXEC_TRACE)
    for (auto* v : stack_)
      std::cout << "| " << v << " ";
    std::cout << std::endl;
    fn->dump_instruction(static_cast<int>(ip - fn->codes()));
#endif

    switch (auto instruction = *ip++; instruction) {
    case OpCode::OP_CONSTANT:
      {
        u8_t constant = _rdbyte();
        push(frame_->get_fun_constant(constant));
      } break;
    case OpCode::OP_NIL: push(nullptr); break;
    case OpCode::OP_POP: pop(); break;
    case OpCode::OP_GET_LOCAL:
      {
        u8_t slot = _rdbyte();
        push(frame_->get_stack_value(slot));
      } break;
    case OpCode::OP_SET_LOCAL:
      {
        u8_t slot = _rdbyte();
        frame_->set_stack_value(slot, peek());
      } break;
    case OpCode::OP_DEF_GLOBAL:
      {
        u8_t constant = _rdbyte();
        auto* key = Xptr::down<StringObject>(frame_->get_fun_constant(constant));
        globals_->set_entry(key, pop());
      } break;
    case OpCode::OP_GET_GLOBAL:
      {
        u8_t constant = _rdbyte();
        auto* key = Xptr::down<StringObject>(frame_->get_fun_constant(constant));
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
        auto* key = Xptr::down<StringObject>(frame_->get_fun_constant(constant));
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
    case OpCode::OP_RETURN:
      // std::cout << pop() << std::endl;
      return true;
    case OpCode::OP_JUMP:
      {
        u16_t offset = _rdword();
        ip += offset;
      } break;
    case OpCode::OP_JUMP_IF_FALSE:
      {
        u16_t offset = _rdword();
        Value cond = peek();
        if (BaseObject::is_nil(cond) || (cond->type() == ObjType::BOOLEAN &&
              !Xptr::down<BooleanObject>(cond)->value())) {
          ip += offset;
        }
      } break;
    case OpCode::OP_LOOP:
      {
        u16_t offset = _rdword();
        ip -= offset;
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
        Value fun = peek(argc);
        if (BaseObject::is_native(fun)) {
          Value ret = Xptr::down<NativeObject>(fun)->get_function()(
              argc, frame_->stack_values(frame_->stack_size() - argc));
          frame_->resize_stack(frame_->stack_size() - argc - 1);
          push(ret);
        }
        else {
          // TODO:
        }
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

  auto* fn = c.compile(*this, source_bytes);
  if (fn == nullptr)
    return InterpretResult::COMPILE_ERROR;
  // fn->dump();
  call(fn);

  // collect();
  // print_stack();

  return run() ? InterpretResult::OK : InterpretResult::RUNTIME_ERROR;
}

}
