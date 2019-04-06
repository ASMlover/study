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

VM::VM(void) {
  globals_ = TableObject::create(*this);

  stack_.push_back(StringObject::create(*this, "print", 5));
  stack_.push_back(NativeObject::create(*this,
        [](int argc, Value* args) -> Value {
          for (int i = 0; i < argc; ++i)
            std::cout << args[i] << " ";
          std::cout << std::endl;
          return nullptr;
        }));
  stack_offset_ += 2;
  globals_->set_entry(Xptr::down<StringObject>(stack_[0]), stack_[1]);
}

VM::~VM(void) {
  globals_ = nullptr;

  for (auto* o : objects_)
    free_object(o);
  objects_.clear();
  gray_stack_.clear();
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  std::cerr << std::endl;
  va_end(ap);

  for (auto i = callframes_.size() - 1; i >= 0; --i) {
    auto& frame = callframes_[i];
    auto instruction_index = static_cast<int>(frame.ip - frame.fun->codes());
    int lineno = frame.fun->get_codeline(instruction_index);
    std::cerr << "[LINE: " << lineno << "]" << std::endl;
  }
}

void VM::append_frame(FunctionObject* fn,
    const u8_t* ip, int stack_begin, bool with_replace) {
  stack_begin += stack_offset_;
  if (with_replace && !callframes_.empty()) {
    auto& frame = callframes_.back();
    frame.assign(fn, ip, stack_begin);
  }
  else {
    callframes_.push_back({fn, ip, stack_begin});
  }
}

std::optional<bool> VM::pop_boolean(void) {
  if (peek()->type() != ObjType::BOOLEAN) {
    runtime_error("operand must be a boolean");
    return {};
  }
  return {Xptr::down<BooleanObject>(pop())->value()};
}

std::optional<double> VM::pop_numeric(void) {
  if (peek()->type() != ObjType::NUMERIC) {
    runtime_error("operand must be a numeric");
    return {};
  }
  return {Xptr::down<NumericObject>(pop())->value()};
}

std::optional<std::tuple<double, double>> VM::pop_numerics(void) {
  if (peek(0)->type() != ObjType::NUMERIC) {
    runtime_error("right operand must be a numeric");
    return {};
  }
  if (peek(1)->type() != ObjType::NUMERIC) {
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

  // mark the roots
  for (auto* v : stack_)
    gray_value(v);

  for (auto& f : callframes_)
    gray_value(f.fun);

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
  for (auto* o : stack_)
    std::cout << i++ << " : " << o << std::endl;
}

bool VM::run(void) {
  auto& frame = callframes_.back();
  const u8_t* ip = frame.ip;

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
        push(frame.fun->get_constant(constant));
      } break;
    case OpCode::OP_NIL: push(nullptr); break;
    case OpCode::OP_POP: pop(); break;
    case OpCode::OP_GET_LOCAL:
      {
        u8_t slot = _rdbyte();
        push(stack_[frame.stack_begin + slot]);
      } break;
    case OpCode::OP_SET_LOCAL:
      {
        u8_t slot = _rdbyte();
        stack_[frame.stack_begin + slot] = peek();
      } break;
    case OpCode::OP_DEF_GLOBAL:
      {
        u8_t constant = _rdbyte();
        auto* key = Xptr::down<StringObject>(frame.fun->get_constant(constant));
        globals_->set_entry(key, pop());
      } break;
    case OpCode::OP_GET_GLOBAL:
      {
        u8_t constant = _rdbyte();
        auto* key = Xptr::down<StringObject>(frame.fun->get_constant(constant));
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
        auto* key = Xptr::down<StringObject>(frame.fun->get_constant(constant));
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
        if (peek(0)->type() == ObjType::STRING
            && peek(1)->type() == ObjType::STRING) {
          auto* b = Xptr::down<StringObject>(pop());
          auto* a = Xptr::down<StringObject>(pop());
          push(StringObject::concat(*this, a, b));
        }
        else if (peek(0)->type() == ObjType::NUMERIC
            && peek(1)->type() == ObjType::NUMERIC) {
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
        if (cond == nullptr || (cond->type() == ObjType::BOOLEAN &&
              !Xptr::down<BooleanObject>(cond)->value())) {
          ip += offset;
        }
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
        Value ret = Xptr::down<NativeObject>(fun)->get_function()(
            argc, &stack_[stack_.size() - argc]);
        stack_.resize(stack_.size() - argc - 1);
        push(ret);
      } break;
    }
  }

  return true;
}

void VM::gray_value(Value v) {
  if (v == nullptr)
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
  append_frame(fn, fn->codes());

  // collect();
  // print_stack();

  return run() ? InterpretResult::OK : InterpretResult::RUNTIME_ERROR;
}

}
