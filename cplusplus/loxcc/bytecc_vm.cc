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
#include <chrono>
#include <cstdarg>
#include <iostream>
#include "bytecc_chunk.hh"
#include "bytecc_vm.hh"

namespace loxcc::bytecc {

class CallFrame final : public Copyable {
  ClosureObject* closure_{};
  const u8_t* ip_{};
  int begpos_{};
public:
  CallFrame(ClosureObject* closure, const u8_t* ip, int begpos = 0) noexcept
    : closure_(closure), ip_(ip), begpos_(begpos) {
  }

  inline ClosureObject* closure(void) const { return closure_; }
  inline const u8_t* ip(void) const { return ip_; }
  inline int begpos(void) const { return begpos_; }

  inline void set_ip(const u8_t* ip) { ip_ = ip; }
  inline u8_t get_ip(int i) const { return ip_[i]; }
  inline u8_t inc_ip(void) { return *ip_++; }
  inline u8_t dec_ip(void) { return *ip_--; }
  inline void add_ip(int offset) { ip_ += offset; }
  inline void sub_ip(int offset) { ip_ -= offset; }

  inline FunctionObject* frame_fn(void) const { return closure_->fn(); }
  inline Chunk* frame_chunk(void) const { return closure_->fn()->chunk(); }
};

VM::VM(void) noexcept {
  define_native("clock", [](int argc, Value* args) -> Value {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
      });

  ctor_string_ = StringObject::create(*this, "ctor");
}

VM::~VM(void) {
  globals_.clear();
  interned_strings_.clear();
  ctor_string_ = nullptr;

  while (!all_objects_.empty()) {
    auto* o = all_objects_.back();
    all_objects_.pop_back();
    free_object(o);
  }
  worked_objects_.clear();
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  fprintf(stderr, "\n");

  int i = Xt::as_type<int>(frames_.size()) - 1;
  for (; i >= 0; --i) {
    auto& frame = frames_[i];
    Chunk* chunk = frame.frame_chunk();
    int ins = Xt::as_type<int>(frame.ip() - chunk->codes()) - 1;

    std::cerr << "[LINE: " << chunk->get_line(ins) << "] in "
      << "`" << frame.frame_fn()->name_astr() << "()`" << std::endl;
  }
  reset();
}

void VM::reset(void) {
  stack_.clear();
  frames_.clear();
  open_upvalues_ = nullptr;
}

Value* VM::stack_values(int distance) {
  return &stack_[stack_.size() - distance];
}

void VM::stack_resize(int distance) {
  stack_.resize(stack_.size() - distance);
}

void VM::set_stack(int distance, const Value& v) {
  stack_[stack_.size() - distance] = v;
}

void VM::push(const Value& value) {
  stack_.push_back(value);
}

Value VM::pop(void) {
  Value value = stack_.back();
  stack_.pop_back();
  return value;
}

const Value& VM::peek(int distance) const {
  return stack_[stack_.size() - 1 - distance];
}

void VM::define_native(const str_t& name, const NativeFn& fn) {
  push(NativeObject::create(*this, fn));
  globals_[name] = peek();
  pop();
}

void VM::define_native(const str_t& name, NativeFn&& fn) {
  push(NativeObject::create(*this, std::move(fn)));
  globals_[name] = peek();
  pop();
}

bool VM::call(ClosureObject* closure, int argc) {
  if (argc != closure->fn()->arity()) {
    runtime_error("`%s` takes %d arguments but %d were given",
        closure->fn()->name_astr(), closure->fn()->arity(), argc);
    return false;
  }

  if (frames_.size() >= kMaxFrames) {
    runtime_error("stack overflow");
    return false;
  }

  frames_.push_back(CallFrame(closure,
        closure->fn()->chunk()->codes(),
        Xt::as_type<int>(stack_.size() - argc - 1)));
  return true;
}

bool VM::call_value(const Value& callee, int argc) {
  if (callee.is_object()) {
    switch (callee.as_object()->type()) {
    case ObjType::NATIVE:
      {
        Value r = callee.as_native()(argc, stack_values(argc));
        stack_resize(argc + 1);
        push(r);
        return true;
      }
    case ObjType::CLOSURE:
      return call(callee.as_closure(), argc);
    case ObjType::CLASS:
      {
        ClassObject* cls = callee.as_class();
        set_stack(argc + 1, InstanceObject::create(*this, cls));
        if (auto ctor = cls->get_method(ctor_string_); ctor) {
          return call((*ctor).as_closure(), argc);
        }
        else if (argc != 0) {
          runtime_error("`%s` takes 0 arguments but %d were given",
              ctor_string_->cstr(), argc);
          return false;
        }
        return true;
      }
    case ObjType::BOUND_METHOD:
      {
        BoundMehtodObject* bound = callee.as_bound_method();
        set_stack(argc + 1, bound->owner());
        call(bound->method(), argc);
      }
    default: break; // do nothing
    }
  }

  runtime_error("can only call functions and classes");
  return false;
}

void VM::free_object(BaseObject* o) {
  // TODO:
}

InterpretRet VM::interpret(const str_t& source_bytes) {
  return InterpretRet::OK;
}

}
