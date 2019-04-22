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

static Value kNil = Value::make_nil();

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
  define_native("clock", [](int argc, Value* args) -> Value {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
      });
  define_native("str", [this](int argc, Value* args) -> Value {
        return Value::make_object(
            StringObject::create(*this, args[0].stringify()));
      });
  ctor_string_ = StringObject::create(*this, "ctor");
}

VM::~VM(void) {
  globals_.clear();
  intern_strings_.clear();
  ctor_string_ = nullptr;

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
  push(StringObject::create(
        *this, name.c_str(), static_cast<int>(name.size())));
  push(NativeObject::create(*this, fn));
  globals_[name] = peek();
  pop();
  pop();
}

void VM::define_native(const str_t& name, NativeFunction&& fn) {
  push(StringObject::create(
        *this, name.c_str(), static_cast<int>(name.size())));
  push(NativeObject::create(*this, std::move(fn)));
  globals_[name] = peek();
  pop();
  pop();
}

void VM::create_class(StringObject* name, ClassObject* superclass) {
  ClassObject* cls = ClassObject::create(*this, name, superclass);
  push(cls);

  // inheritance methods of superclasses
  if (superclass != nullptr)
    cls->inherit_from(superclass);
}

void VM::define_method(StringObject* name) {
  const Value& method = peek(0);
  ClassObject* klass = peek(1).as_class();
  klass->bind_method(name, method);
  pop();
}

bool VM::bind_method(ClassObject* cls, StringObject* name) {
  if (auto method = cls->get_method(name); method) {
    auto* bound = BoundMethodObject::create(
        *this, peek(0), (*method).as_closure());
    pop(); // pop instance
    push(bound);
    return true;
  }

  runtime_error("undefined property `%s`", name->chars());
  return false;
}

void VM::reset_stack(void) {
  stack_.clear();
  frames_.clear();
  open_upvalues_ = nullptr;
}

void VM::push(const Value& val) {
  stack_.push_back(val);
}

Value VM::pop(void) {
  Value val = stack_.back();
  stack_.pop_back();
  return val;
}

Value& VM::peek(int distance) {
  return stack_.empty() ? kNil : stack_[stack_.size() - 1 - distance];
}

const Value& VM::peek(int distance) const {
  return stack_.empty() ? kNil : stack_[stack_.size() - 1 - distance];
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  std::cerr << std::endl;

  for (auto i = static_cast<int>(frames_.size() - 1); i >= 0; --i) {
    auto& frame = frames_[i];
    auto* fn = frame.closure()->get_function();
    auto offset = static_cast<int>(frame.ip() - frame.get_closure_codes());
    std::cerr
      << "[LINE: " << frame.get_closure_codeline(offset) << "] in "
      << fn->name()->chars() << std::endl;
  }

  reset_stack();
}

std::optional<bool> VM::pop_boolean(void) {
  if (!peek().is_boolean()) {
    runtime_error("operand must be a boolean");
    return {};
  }
  return {pop().as_boolean()};
}

std::optional<double> VM::pop_numeric(void) {
  if (!peek().is_numeric()) {
    runtime_error("operand must be a numeric");
    return {};
  }
  return {pop().as_numeric()};
}

std::optional<std::tuple<double, double>> VM::pop_numerics(void) {
  if (!peek(0).is_numeric() || !peek(1).is_numeric()) {
    runtime_error("operands must be numerics");
    return {};
  }

  auto b = pop().as_numeric();
  auto a = pop().as_numeric();
  return {{a, b}};
}

void VM::collect(void) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "********* collect: starting *********" << std::endl;
#endif

  // mark the stack roots
  for (auto& v : stack_)
    gray_value(v);
  for (auto& f : frames_)
    gray_object(f.closure());

  // mark the openvalues
  for (auto* upvalue = open_upvalues_;
      upvalue != nullptr; upvalue = upvalue->next()) {
    gray_object(upvalue);
  }

  gray_table(*this, globals_);
  gray_compiler_roots(*this);
  gray_object(ctor_string_);

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

  // adjust the heap size head on live memory
  next_gc_ = bytes_allocated_ * 2;

#if defined(DEBUG_GC_TRACE)
  std::cout << "********* collect: finished *********" << std::endl;
#endif
}

void VM::remove_undark_intern_strings(void) {
  for (auto it = intern_strings_.begin(); it != intern_strings_.end();) {
    if (!it->second->is_dark())
      intern_strings_.erase(it++);
    else
      ++it;
  }
}

bool VM::call_closure(ClosureObject* closure, int argc) {
  if (argc < closure->get_function()->arity()) {
    runtime_error("not enough arguments");
    return false;
  }

  frames_.emplace_back(CallFrame(closure,
        closure->get_function()->codes(),
        static_cast<int>(stack_.size() - argc - 1)));
  return true;
}

bool VM::call(const Value& callee, int argc/* = 0*/) {
  if (callee.is_object()) {
    switch (obj_type(callee)) {
    case ObjType::BOUND_METHOD:
      {
        BoundMethodObject* bound = callee.as_bound_method();
        stack_[stack_.size() - argc - 1] = bound;
        return call_closure(bound->method(), argc);
      }
    case ObjType::CLASS:
      {
        ClassObject* cls = callee.as_class();

        // create the instance
        stack_[stack_.size() - argc - 1] = InstanceObject::create(*this, cls);
        // call the constructor if there is one
        if (auto ctor = cls->get_method(ctor_string_); ctor)
          return call_closure((*ctor).as_closure(), argc);
        // no constructor, just discard the arguments
        stack_.resize(stack_.size() - argc);
        return true;
      }
    case ObjType::CLOSURE:
      return call_closure(callee.as_closure(), argc);
    case ObjType::NATIVE:
      {
        Value* args = argc > 0 ? &stack_[stack_.size() - argc] : nullptr;
        Value ret = callee.as_native()(argc, args);
        stack_.resize(stack_.size() - argc - 1);
        push(ret);
        return true;
      }
    default: break;
    }
  }

  runtime_error("can only call functions and classes");
  return false;
}

bool VM::invoke_from_class(ClassObject* cls, StringObject* name, int argc) {
  if (auto method = cls->get_method(name); method)
    return call_closure((*method).as_closure(), argc);

  runtime_error("undefined property `%s`", name->chars());
  return false;
}

bool VM::invoke(StringObject* name, int argc) {
  const Value& receiver = peek(argc);
  if (!receiver.is_instance()) {
    runtime_error("only instances have methods");
    return false;
  }

  InstanceObject* inst = receiver.as_instance();
  if (auto callee = inst->get_field(name); callee) {
    stack_[stack_.size() - argc] = *callee;
    return call(*callee, argc);
  }

  return invoke_from_class(inst->get_class(), name, argc);
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
  auto _rdstring = [_rdconstant](void) -> StringObject* {
    return _rdconstant().as_string();
  };

#define BINARY_OP(op) do {\
  if (!peek(0).is_numeric() || !peek(1).is_numeric()) {\
    runtime_error("operands must be numerics");\
    return false;\
  }\
  double b = pop().as_numeric();\
  double a = pop().as_numeric();\
  push(a op b);\
} while (false)

  for (;;) {
#if defined(DEBUG_EXEC_TRACE)
    {
      for (auto& v : stack_)
        std::cout << "| " << v << " ";
      std::cout << std::endl;
      auto* fn = frame->closure()->get_function();
      fn->disassemble_instruction(
          static_cast<int>(frame->ip() - frame->get_closure_codes()));
    }
#endif

    switch (auto instruction = _rdbyte(); instruction) {
    case OpCode::OP_CONSTANT: push(_rdconstant()); break;
    case OpCode::OP_NIL: push(nullptr); break;
    case OpCode::OP_TRUE: push(true); break;
    case OpCode::OP_FALSE: push(false); break;
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
        auto* key = _rdstring();
        globals_[key->chars()] = pop();
      } break;
    case OpCode::OP_GET_GLOBAL:
      {
        auto* key = _rdstring();
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
        auto* key = _rdstring();
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
        if (!peek().is_instance()) {
          runtime_error("only instances have faileds");
          return false;
        }

        // class fields
        auto* inst = peek().as_instance();
        auto* name = _rdstring();
        if (auto val = inst->get_field(name); val) {
          pop(); // pop out instance
          push(*val);
          break;
        }

        if (!bind_method(inst->get_class(), name))
          return false;
      } break;
    case OpCode::OP_SET_FIELD:
      {
        if (!peek(1).is_instance()) {
          runtime_error("only instances have fields");
          return false;
        }

        // class fields
        auto* inst = peek(1).as_instance();
        inst->set_field(_rdstring(), peek(0));
        Value val = pop();
        pop();
        push(val);
      } break;
    case OpCode::OP_GET_SUPER:
      {
        StringObject* name = _rdstring();
        ClassObject* superclass = pop().as_class();
        if (!bind_method(superclass, name))
          return false;
      } break;
    case OpCode::OP_EQ:
      {
        auto b = pop();
        auto a = pop();
        push(a == b);
      } break;
    case OpCode::OP_NE:
      {
        auto b = pop();
        auto a = pop();
        push(a != b);
      } break;
    case OpCode::OP_GT:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(a > b);
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_GE:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(a >= b);
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_LT:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(a < b);
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_LE:
      if (auto r = pop_numerics(); r) {
        auto [a, b] = *r;
        push(a <= b);
      }
      else {
        return false;
      }
      break;
    case OpCode::OP_ADD:
      {
        if (peek(0).is_string() && peek(1).is_string()) {
          auto* b = pop().as_string();
          auto* a = pop().as_string();
          push(StringObject::concat(*this, a, b));
        }
        else if (peek(1).is_numeric() && peek(1).is_numeric()) {
          double b = pop().as_numeric();
          double a = pop().as_numeric();
          push(a + b);
        }
        else {
          runtime_error("operands must be two strings or two numerics");
          return false;
        }
      } break;
    case OpCode::OP_SUB: BINARY_OP(-); break;
    case OpCode::OP_MUL: BINARY_OP(*); break;
    case OpCode::OP_DIV: BINARY_OP(/); break;
    case OpCode::OP_NOT:
      {
        auto b = pop().is_falsely();
        push(b);
      } break;
    case OpCode::OP_NEG:
      {
        if (!peek(0).is_numeric()) {
          runtime_error("operand must be a numeric");
          return false;
        }
        push(-pop().as_numeric());
      } break;
    case OpCode::OP_JUMP:
      {
        u16_t offset = _rdword();
        frame->add_ip(offset);
      } break;
    case OpCode::OP_JUMP_IF_FALSE:
      {
        u16_t offset = _rdword();
        if (peek().is_falsely())
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
        StringObject* method = _rdstring();
        int argc = instruction - OpCode::OP_INVOKE_0;
        if (!invoke(method, argc))
          return false;
        frame = &frames_.back();
      } break;
    case OpCode::OP_SUPER_0:
    case OpCode::OP_SUPER_1:
    case OpCode::OP_SUPER_2:
    case OpCode::OP_SUPER_3:
    case OpCode::OP_SUPER_4:
    case OpCode::OP_SUPER_5:
    case OpCode::OP_SUPER_6:
    case OpCode::OP_SUPER_7:
    case OpCode::OP_SUPER_8:
      {
        StringObject* method = _rdstring();
        int argc = instruction - OpCode::OP_SUPER_0;
        ClassObject* superclass = pop().as_class();
        if (!invoke_from_class(superclass, method, argc))
          return false;
        frame = &frames_.back();
      } break;
    case OpCode::OP_CLOSURE:
      {
        auto* fn = _rdconstant().as_function();
        auto* closure = ClosureObject::create(*this, fn);
        push(closure);

        for (int i = 0; i < closure->upvaules_count(); ++i) {
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
        if (stack_.empty() || frame->stack_start() >= stack_.size())
          close_upvalues(nullptr);
        else
          close_upvalues(&stack_[frame->stack_start()]);

        frames_.pop_back();
        if (frames_.size() == 0)
          return true;
        stack_.resize(frame->stack_start());
        push(ret);

        frames_.pop_back();
        frame = &frames_.back();
      } break;
    case OpCode::OP_CLASS:
      create_class(_rdstring(), nullptr); break;
    case OpCode::OP_SUBCLASS:
      {
        const Value& superclass = peek(0);
        if (!superclass.is_class()) {
          runtime_error("superclass must be a class");
          return false;
        }
        create_class(_rdstring(), superclass.as_class());
      } break;
    case OpCode::OP_METHOD:
      define_method(_rdstring()); break;
    }
  }

#undef BINARY_OP

  return true;
}

void VM::append_object(BaseObject* o) {
  bytes_allocated_ += o->size_bytes();
  if (bytes_allocated_ > next_gc_)
    collect();

  objects_.push_back(o);
}

void VM::set_intern_string(u32_t hash, StringObject* s) {
  // push StringObject `s` into stack, make sure it not be collected
  push(s);
  intern_strings_[hash] = s;
  pop();
}

std::optional<StringObject*> VM::get_intern_string(u32_t hash) const {
  if (auto it = intern_strings_.find(hash); it != intern_strings_.end())
    return {Xptr::down<StringObject>(it->second)};
  return {};
}

void VM::gray_object(BaseObject* obj) {
  if (obj == nullptr)
    return;

  if (obj->is_dark())
    return;
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << Xptr::address(obj) << "` gray " << obj << std::endl;
#endif
  obj->set_dark(true);
  gray_stack_.push_back(obj);
}

void VM::gray_value(const Value& v) {
  if (!v.is_object())
    return;
  gray_object(v.as_object());
}

void VM::blacken_object(BaseObject* obj) {
#if defined(DEBUG_GC_TRACE)
  std::cout << "`" << Xptr::address(obj) << "` blacken " << obj << std::endl;
#endif
  obj->blacken(*this);
}

void VM::free_object(BaseObject* obj) {
#if defined(DEBUG_GC_TRACE)
  std::cout
    << "`" << Xptr::address(obj) << "` free "
    << obj->type_name() << " " << obj << std::endl;
#endif
  delete obj;
}

InterpretResult VM::interpret(const str_t& source_bytes) {
  Compile c;

  // FIXME: execute from cli will crash ?
  stack_.clear();
  frames_.clear();

  auto* fn = c.compile(*this, source_bytes);
  if (fn == nullptr)
    return InterpretResult::COMPILE_ERROR;

  push(fn);
  auto* closure = ClosureObject::create(*this, fn);
  pop();

  call(closure, 0);

  return run() ? InterpretResult::OK : InterpretResult::RUNTIME_ERROR;
}

}
