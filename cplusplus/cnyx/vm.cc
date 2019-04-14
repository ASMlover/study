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
#include <chrono>
#include <cstdarg>
#include <iostream>
#include <sstream>
#include "compile.hh"
#include "vm.hh"

namespace nyx {

class CallFrame : private UnCopyable {
  using IterCallback = std::function<void(FunctionObject*, const u8_t*)>;

  ClosureObject* closure_{};
  u8_t* ip_{};
  int stack_start_{};
public:
  CallFrame(ClosureObject* closure, u8_t* ip, int start)
    : closure_(closure), ip_(ip), stack_start_(start) {
  }

  CallFrame(CallFrame&& frame) noexcept
    : closure_(std::move(frame.closure_))
    , ip_(std::move(frame.ip_))
    , stack_start_(std::move(frame.stack_start_)) {
  }

  inline ClosureObject* closure(void) const { return closure_; }
  inline u8_t* ip(void) const { return ip_; }
  inline void set_ip(u8_t* ip) { ip_ = ip; }
  inline u8_t get_ip(int i) { return ip_[i]; }
  inline u8_t inc_ip(void) { return *ip_++; }
  inline u8_t dec_ip(void) { return *ip_--; }
  inline void add_ip(int offset) { ip_ += offset; }
  inline void sub_ip(int offset) { ip_ -= offset; }
  inline int stack_start(void) const { return stack_start_; }

  inline const u8_t* get_closure_codes(void) const {
    return closure_->get_function()->codes();
  }

  inline u8_t get_closure_code(int i) const {
    return closure_->get_function()->get_code(i);
  }

  inline int get_closure_codeline(int i) const {
    return closure_->get_function()->get_codeline(i);
  }

  inline Value get_closure_constant(int i) const {
    return closure_->get_function()->get_constant(i);
  }
};

VM::VM(void) {
  // define native functions
  define_native("print", [](int argc, Value* args) -> Value {
        for (int i = 0; i < argc; ++i)
          std::cout << args[i] << " ";
        std::cout << std::endl;
        return nullptr;
      });
  define_native("clock", [this](int argc, Value* args) -> Value {
        double sec = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
        return NumericObject::create(*this, sec);
      });
}

VM::~VM(void) {
  globals_.clear();

  // new object pushed into `objects_` list may has reference to old object,
  // so need free newly object first of `objects_` list.
  while (!objects_.empty()) {
    auto* o = objects_.back();
    objects_.pop_back();
    free_object(o);
  }
  gray_stack_.clear();
}

void VM::define_native(const str_t& name, const NativeFunction& fn) {
  globals_[name] = NativeObject::create(*this, fn);
}

void VM::define_native(const str_t& name, NativeFunction&& fn) {
  globals_[name] = NativeObject::create(*this, std::move(fn));
}

void VM::create_class(StringObject* name, ClassObject* superclass) {
  push(ClassObject::create(*this, name, superclass));
}

void VM::bind_method(StringObject* name) {
  Value method = peek(0);
  ClassObject* klass = Xptr::down<ClassObject>(peek(1));

  klass->bind_method(name, method);
  pop();
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
  // return stack_[stack_.size() - 1 - distance];
  return stack_.empty() ? nullptr : stack_[stack_.size() - 1 - distance];
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  std::cerr << std::endl;

  for (auto i = frames_.size() - 1; i >= 0; --i) {
    auto& frame = frames_[i];
    auto offset = static_cast<int>(frame.ip() - frame.get_closure_codes());
    std::cerr << "[LINE: " << frame.get_closure_codeline(offset) << "]" << std::endl;
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
  if (!BaseObject::is_numeric(peek(0)) || !BaseObject::is_numeric(peek(1))) {
    runtime_error("operands must be numerics");
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
    gray_value(f.closure());

  // mark the openvalues
  for (auto* upvalue = open_upvalues_;
      upvalue != nullptr; upvalue = upvalue->next()) {
    gray_value(upvalue);
  }

  gray_table(*this, globals_);
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

bool VM::call_closure(ClosureObject* closure, int argc) {
  if (argc < closure->get_function()->arity()) {
    runtime_error("not enough arguments");
    return false;
  }

  frames_.emplace_back(CallFrame(closure,
        closure->get_function()->codes(),
        static_cast<int>(stack_.size() - argc)));
  return true;
}

bool VM::call(Value callee, int argc/* = 0*/) {
  if (BaseObject::is_class(callee)) {
    InstanceObject* inst = InstanceObject::create(
        *this, Xptr::down<ClassObject>(callee));
    stack_[stack_.size() - argc - 1] = inst;
    stack_.resize(stack_.size() - argc);
    return true;
  }
  if (BaseObject::is_closure(callee)) {
    return call_closure(Xptr::down<ClosureObject>(callee), argc);
  }
  if (BaseObject::is_native(callee)) {
    Value* args{};
    if (argc > 0)
      args = &stack_[stack_.size() - argc];
    Value ret = Xptr::down<NativeObject>(callee)->get_function()(argc, args);
    stack_.resize(stack_.size() - argc - 1);
    push(ret);
    return true;
  }

  runtime_error("can only call functions and classes");
  return false;
}

bool VM::invoke(Value receiver, StringObject* name, int argc) {
  if (!BaseObject::is_instance(receiver)) {
    runtime_error("only instances have methods");
    return false;
  }

  InstanceObject* inst = Xptr::down<InstanceObject>(receiver);
  if (auto callee = inst->get_field(name); callee) {
    stack_[stack_.size() - argc - 1] = *callee;
    return call(*callee, argc);
  }
  ClassObject* cls = inst->get_class();
  while (cls != nullptr) {
    if (auto method = cls->get_method(name); method) {
      return call_closure(Xptr::down<ClosureObject>(*method), argc);
    }
    cls = cls->superclass();
  }

  runtime_error("%s does not implement `%s`",
      inst->get_class()->name()->chars(), name->chars());
  return false;
}

UpvalueObject* VM::capture_upvalue(Value* local) {
  if (open_upvalues_ == nullptr) {
    open_upvalues_ = UpvalueObject::create(*this, local);
    return open_upvalues_;
  }

  UpvalueObject* prev_upvalue{};
  UpvalueObject* upvalue = open_upvalues_;

  while (upvalue != nullptr && upvalue->value() > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next();
  }
  if (upvalue != nullptr && upvalue->value() == local)
    return upvalue;

  auto* created_upvalue = UpvalueObject::create(*this, local);
  created_upvalue->set_next(upvalue);

  if (prev_upvalue == nullptr)
    open_upvalues_ = created_upvalue;
  else
    prev_upvalue->set_next(created_upvalue);

  return created_upvalue;
}

void VM::close_upvalues(Value* last) {
  while (open_upvalues_ != nullptr && open_upvalues_->value() >= last) {
    auto* upvalue = open_upvalues_;

    upvalue->set_closed(*upvalue->value());
    auto closed = upvalue->closed();
    upvalue->set_value(&closed);

    open_upvalues_ = upvalue->next();
  }
}

bool VM::run(void) {
  auto* frame = &frames_.back();

  auto _rdbyte = [&frame](void) -> u8_t { return frame->inc_ip(); };
  auto _rdword = [&frame](void) -> u16_t {
    return (frame->add_ip(2),
        static_cast<u16_t>((frame->get_ip(-2) << 8) | frame->get_ip(-1)));
  };
  auto _rdconstant = [&frame, _rdbyte](void) -> Value {
    return frame->get_closure_constant(_rdbyte());
  };
  auto _rdsymbol = [_rdconstant](void) -> StringObject* {
    return Xptr::down<StringObject>(_rdconstant());
  };

  for (;;) {
#if defined(DEBUG_EXEC_TRACE)
    {
      for (auto* o : stack_)
        std::cout << "| " << o << " ";
      std::cout << std::endl;
      auto* fn = frame->closure()->get_function();
      fn->dump_instruction(
          static_cast<int>(frame->ip() - frame->get_closure_codes()));
    }
#endif

    switch (auto instruction = frame->inc_ip(); instruction) {
    case OpCode::OP_CONSTANT: push(_rdconstant()); break;
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
        auto* key = _rdsymbol();
        globals_[key->chars()] = pop();
      } break;
    case OpCode::OP_GET_GLOBAL:
      {
        auto* key = _rdsymbol();
        if (auto it = globals_.find(key->chars()); it != globals_.end()) {
          push(it->second);
        }
        else {
          runtime_error("undefined variable `%s`", key->chars());
          return false;
        }
      } break;
    case OpCode::OP_SET_GLOBAL:
      {
        auto* key = _rdsymbol();
        if (auto it = globals_.find(key->chars()); it != globals_.end()) {
          globals_[key->chars()] = peek();
        }
        else {
          runtime_error("undefined variable `%s`", key->chars());
          return false;
        }
      } break;
    case OpCode::OP_GET_UPVALUE:
      {
        u8_t slot = _rdbyte();
        push(*frame->closure()->get_upvalue(slot)->value());
      } break;
    case OpCode::OP_SET_UPVALUE:
      {
        u8_t slot = _rdbyte();
        Value value = pop();
        frame->closure()->get_upvalue(slot)->set_value(&value);
      } break;
    case OpCode::OP_GET_FIELD:
      {
        if (!BaseObject::is_instance(peek())) {
          runtime_error("only instances have faileds");
          return false;
        }

        // class fields
        auto* inst = Xptr::down<InstanceObject>(pop());
        auto* name = _rdsymbol();
        if (auto val = inst->get_field(name); val) {
          push(*val);
        }
        else {
          runtime_error("undefined field `%s`", name->chars());
          return false;
        }
      } break;
    case OpCode::OP_SET_FIELD:
      {
        if (!BaseObject::is_instance(peek(1))) {
          runtime_error("only instances have fields");
          return false;
        }

        // class fields
        auto* inst = Xptr::down<InstanceObject>(peek(1));
        inst->set_field(_rdsymbol(), peek(0));
        Value val = pop();
        pop();
        push(val);
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
          runtime_error("operands must be two strings or two numerics");
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
        auto b = BaseObject::is_falsely(pop());
        push(BooleanObject::create(*this, b));
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
        if (BaseObject::is_falsely(peek()))
          frame->add_ip(offset);
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
        if (!call(peek(argc), argc))
          return false;
        frame = &frames_.back();
      } break;
    case OpCode::OP_INVOKE_0:
    case OpCode::OP_INVOKE_1:
    case OpCode::OP_INVOKE_2:
    case OpCode::OP_INVOKE_3:
    case OpCode::OP_INVOKE_4:
    case OpCode::OP_INVOKE_5:
    case OpCode::OP_INVOKE_6:
    case OpCode::OP_INVOKE_7:
    case OpCode::OP_INVOKE_8:
      {
        StringObject* method = _rdsymbol();
        int argc = instruction - OpCode::OP_INVOKE_0;
        if (!invoke(peek(argc), method, argc))
          return false;
        frame = &frames_.back();
      } break;
    case OpCode::OP_CLOSURE:
      {
        auto* fn = Xptr::down<FunctionObject>(_rdconstant());
        auto* closure = ClosureObject::create(*this, fn);
        push(closure);

        for (int i = 0; i < fn->upvalues_count(); ++i) {
          u8_t is_local = _rdbyte();
          u8_t index = _rdbyte();
          if (is_local) {
            auto* local = &stack_[frame->stack_start() + index];
            closure->set_upvalue(i, capture_upvalue(local));
          }
          else {
            closure->set_upvalue(i, frame->closure()->get_upvalue(index));
          }
        }
      } break;
    case OpCode::OP_CLOSE_UPVALUE:
      {
        close_upvalues(&stack_.back());
        pop();
      } break;
    case OpCode::OP_RETURN:
      {
        Value ret = pop();
        if (!stack_.empty())
          close_upvalues(&stack_[frame->stack_start() - 1]);

        if (frames_.size() == 1)
          return true;

        stack_.resize(frame->stack_start() - 1);
        push(ret);

        frames_.pop_back();
        frame = &frames_.back();
      } break;
    case OpCode::OP_CLASS:
      create_class(_rdsymbol(), nullptr); break;
    case OpCode::OP_SUBCLASS:
      {
        Value superclass = pop();
        if (!BaseObject::is_class(superclass)) {
          runtime_error("superclass must be a class");
          return false;
        }
        create_class(_rdsymbol(), Xptr::down<ClassObject>(superclass));
      } break;
    case OpCode::OP_METHOD:
      bind_method(_rdsymbol()); break;
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

  push(fn);
  auto* closure = ClosureObject::create(*this, fn);
  pop();
  push(closure);

  call(closure, 0);

  // collect();
  // print_stack();

  return run() ? InterpretResult::OK : InterpretResult::RUNTIME_ERROR;
}

}
