// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <cstdarg>
#include <algorithm>
#include <iostream>
#include "chunk.hh"
#include "compiler.hh"
#include "vm.hh"

namespace tadpole {

class CallFrame final : public Copyable {
  ClosureObject* closure_{};
  const u8_t* ip_{};
  sz_t stack_begpos_{};
public:
  CallFrame() noexcept {}
  CallFrame(ClosureObject* c, const u8_t* i, sz_t begpos = 0) noexcept
    : closure_(c), ip_(i), stack_begpos_(begpos) {
  }

  inline ClosureObject* closure() const noexcept { return closure_; }
  inline FunctionObject* frame_fn() const noexcept { return closure_->fn(); }
  inline Chunk* frame_chunk() const noexcept { return frame_fn()->chunk(); }
  inline const u8_t* ip() const noexcept { return ip_; }
  inline u8_t get_ip(sz_t i) const noexcept { return ip_[i]; }
  inline u8_t inc_ip() noexcept { return *ip_++; }
  inline u8_t dec_ip() noexcept { return *ip_--; }
  inline sz_t stack_begpos() const noexcept { return stack_begpos_; }
};

VM::VM() noexcept {
  gcompiler_ = new GlobalCompiler();
  stack_.reserve(kDefaultCap);

  // TODO: register built-in functions
}

VM::~VM() {
  delete gcompiler_;

  globals_.clear();
  interned_strings_.clear();

  while (!objects_.empty()) {
    auto* o = objects_.back();
    objects_.pop_back();

    reclaim_object(o);
  }
}

void VM::define_native(const str_t& name, NativeFn&& fn) {
  globals_[name] = NativeObject::create(*this, std::move(fn));
}

void VM::append_object(BaseObject* o) {
  if (objects_.size() >= kGCThreshold)
    collect();

  objects_.push_back(o);
}

void VM::mark_object(BaseObject* o) {
  if (o == nullptr || o->is_marked())
    return;

#if defined(_TADPOLE_DEBUG_GC)
  std::cout << "[" << o << "] mark object: `" << o->stringify() << "`" << std::endl;
#endif

  o->set_marked(true);
  worklist_.push_back(o);
}

void VM::mark_value(const Value& v) {
  if (v.is_object())
    mark_object(v.as_object());
}

InterpretRet VM::interpret(const str_t& source_bytes) {
  FunctionObject* fn = gcompiler_->compile(*this, source_bytes);
  if (fn == nullptr)
    return InterpretRet::ECOMPILE;

  push(fn);
  ClosureObject* closure = ClosureObject::create(*this, fn);
  pop();
  call(closure, 0);

  return run();
}

void VM::collect() {
  // mark root objects
  gcompiler_->mark_compiler();
  for (auto& v : stack_)
    mark_value(v);
  for (auto& f : frames_)
    mark_object(f.closure());
  for (auto& g : globals_)
    mark_value(g.second);
  for (auto* u = open_upvalues_; u != nullptr; u = u->next())
    mark_object(u);

  // mark reference objects
  while (!worklist_.empty()) {
    auto* o = worklist_.back();
    worklist_.pop_back();
    o->gc_blacken(*this);
  }

  // delete unmarked interned string objects
  for (auto it = interned_strings_.begin(); it != interned_strings_.end();) {
    if (!it->second->is_marked())
      interned_strings_.erase(it++);
    else
      ++it;
  }

  // sweep and reclaim unmarked objects
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->is_marked()) {
      reclaim_object(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_marked(false);
      ++it;
    }
  }

  gc_threshold_ = std::max(kGCThreshold, objects_.size() * kGCFactor);
}

void VM::reclaim_object(BaseObject* o) {
#if defined(_TADPOLE_DEBUG_GC)
  std::cout << "[" << o << "] reclaim object: `" << o->stringify() << "`" << std::endl;
#endif

  delete o;
}

void VM::reset() {
  stack_.clear();
  frames_.clear();
  open_upvalues_ = nullptr;
}

void VM::runtime_error(const char* format, ...) {
  std::cerr << "Traceback (most recent call last):" << std::endl;
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto& frame = *it;

    sz_t i = frame.frame_chunk()->offset(frame.ip()) - 1;
    std::cerr
      << "  [LINE: " << frame.frame_chunk()->get_line(i) << "] in "
      << "`" << frame.frame_fn()->name_asstr() << "()`"
      << std::endl;
  }

  va_list ap;
  va_start(ap, format);
  std::vfprintf(stderr, format, ap);
  va_end(ap);
  std::fprintf(stderr, "\n");

  reset();
}

void VM::push(Value value) noexcept {
  stack_.push_back(value);
}

Value VM::pop() noexcept {
  Value value = stack_.back();
  stack_.pop_back();
  return value;
}

const Value& VM::peek(sz_t distance) const noexcept {
  return stack_[stack_.size() - 1 - distance];
}

bool VM::call(ClosureObject* closure, sz_t argc) {
  FunctionObject* fn = closure->fn();
  if (fn->arity() != argc) {
    runtime_error("%s() takes exactly %ud arguments (%ud given)",
        fn->name_asstr(), fn->arity(), argc);
    return false;
  }

  frames_.push_back(CallFrame(closure, fn->chunk()->codes(), stack_.size() - argc - 1));
  return true;
}

bool VM::call(const Value& callee, sz_t argc) {
  if (callee.is_object()) {
    switch (callee.objtype()) {
    case ObjType::NATIVE:
      {
        Value* args = nullptr;
        if (argc > 0 && stack_.size() > argc)
          args = &stack_[stack_.size() - argc];

        Value result = callee.as_native()->fn()(argc, args);
        stack_.resize(stack_.size() - argc - 1);
        push(result);

        return true;
      }
    case ObjType::CLOSURE: return call(callee.as_closure(), argc);
    }
  }

  runtime_error("can only call function");
  return false;
}

UpvalueObject* VM::capture_upvalue(Value* local) {
  if (open_upvalues_ == nullptr) {
    open_upvalues_ = UpvalueObject::create(*this, local);
    return open_upvalues_;
  }

  UpvalueObject* upvalue = open_upvalues_;
  UpvalueObject* prev_upvalue = nullptr;
  while (upvalue != nullptr && upvalue->value() > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next();
  }
  if (upvalue != nullptr && upvalue->value() == local)
    return upvalue;

  UpvalueObject* new_upvalue = UpvalueObject::create(*this, local, upvalue);
  if (prev_upvalue == nullptr)
    open_upvalues_ = new_upvalue;
  else
    prev_upvalue->set_next(new_upvalue);
  return new_upvalue;
}

void VM::close_upvalues(Value* last) {
  while (open_upvalues_ != nullptr && open_upvalues_->value() >= last) {
    UpvalueObject* upvalue = open_upvalues_;
    upvalue->set_closed(upvalue->value_asref());
    upvalue->set_value(upvalue->closed_asptr());

    open_upvalues_ = upvalue->next();
  }
}

InterpretRet VM::run() {
  CallFrame* frame = &frames_.back();

#define _RDBYTE()   frame->inc_ip()
#define _RDCONST()  frame->frame_chunk()->get_constant(_RDBYTE())
#define _RDSTR()    _RDCONST().as_string()
#define _RDCSTR()   _RDCONST().as_cstring()
#define _BINOP(op)  do {\
    if (!peek(0).is_numeric() || !peek(1).is_numeric()) {\
      runtime_error("operands must be two numerics");\
      return InterpretRet::ERUNTIME;\
    }\
    double b = pop().as_numeric();\
    double a = pop().as_numeric();\
    push(a op b);\
  } while (false)

  for (;;) {
#if defined(_TADPOLE_DEBUG_VM)
    auto* frame_chunk = frame->frame_chunk();
    std::cout << "          ";
    for (auto& v : stack_)
      std::cout << "[" << v << "]";
    std::cout << std::endl;
    frame_chunk->dis_code(frame_chunk->offset(frame->ip()));
#endif

    switch (Code c = as_type<Code>(_RDBYTE())) {
    case Code::CONSTANT: push(_RDCONST()); break;
    case Code::NIL: push(nullptr); break;
    case Code::FALSE: push(false); break;
    case Code::TRUE: push(true); break;
    case Code::POP: pop(); break;
    case Code::DEF_GLOBAL:
      {
        const char* name = _RDCSTR();
        if (auto it = globals_.find(name); it != globals_.end()) {
          runtime_error("name `%s` is redefined", name);
          return InterpretRet::ERUNTIME;
        }
        else {
          globals_[name] = pop();
        }
      } break;
    case Code::GET_GLOBAL:
      {
        const char* name = _RDCSTR();
        if (auto it = globals_.find(name); it != globals_.end()) {
          push(it->second);
        }
        else {
          runtime_error("name `%s` is not defined", name);
          return InterpretRet::ERUNTIME;
        }
      } break;
    case Code::SET_GLOBAL:
      {
        const char* name = _RDCSTR();
        if (auto it = globals_.find(name); it != globals_.end()) {
          it->second = peek();
        }
        else {
          runtime_error("name `%s` is not defined", name);
          return InterpretRet::ERUNTIME;
        }
      } break;
    case Code::GET_LOCAL: push(stack_[frame->stack_begpos() + _RDBYTE()]); break;
    case Code::SET_LOCAL: stack_[frame->stack_begpos() + _RDBYTE()] = peek(); break;
    case Code::GET_UPVALUE:
      {
        u8_t slot = _RDBYTE();
        push(frame->closure()->get_upvalue(slot)->value_asref());
      } break;
    case Code::SET_UPVALUE:
      {
        u8_t slot = _RDBYTE();
        frame->closure()->get_upvalue(slot)->set_value(peek());
      } break;
    case Code::ADD:
      {
        if (peek(0).is_string() && peek(1).is_string()) {
          StringObject* b = peek(0).as_string();
          StringObject* a = peek(1).as_string();
          StringObject* s = StringObject::concat(*this, a, b);
          pop();
          pop();
          push(s);
        }
        else if (peek(0).is_numeric() && peek(1).is_numeric()) {
          double b = pop().as_numeric();
          double a = pop().as_numeric();
          push(a + b);
        }
        else {
          runtime_error("operands must be two strings or two numerics");
          return InterpretRet::ERUNTIME;
        }
      } break;
    case Code::SUB: _BINOP(-); break;
    case Code::MUL: _BINOP(*); break;
    case Code::DIV: _BINOP(/); break;
    case Code::CALL_0:
    case Code::CALL_1:
    case Code::CALL_2:
    case Code::CALL_3:
    case Code::CALL_4:
    case Code::CALL_5:
    case Code::CALL_6:
    case Code::CALL_7:
    case Code::CALL_8:
      {
        sz_t argc = c - Code::CALL_0;
        if (!call(peek(argc), argc))
          return InterpretRet::ERUNTIME;
        frame = &frames_.back();
      } break;
    case Code::CLOSURE:
      {
        FunctionObject* fn = _RDCONST().as_function();
        ClosureObject* closure = ClosureObject::create(*this, fn);
        push(closure);

        for (sz_t i = 0; i < fn->upvalues_count(); ++i) {
          u8_t is_local = _RDBYTE();
          u8_t index = _RDBYTE();
          if (is_local)
            closure->set_upvalue(i, capture_upvalue(&stack_[frame->stack_begpos() + index]));
          else
            closure->set_upvalue(i, frame->closure()->get_upvalue(index));
        }
      } break;
    }
  }

#undef _BINOP
#undef _RDCSTR
#undef _RDSTR
#undef _RDCONST
#undef _RDBYTE

  return InterpretRet::OK;
}

}
