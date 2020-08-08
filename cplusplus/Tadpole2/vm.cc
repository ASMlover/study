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
}

VM::~VM() {
}

void VM::define_native(const str_t& name, NativeFn&& fn) {
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
  return InterpretRet::OK;
}

void VM::collect() {
}

void VM::reclaim_object(BaseObject* o) {
#if defined(_TADPOLE_DEBUG_GC)
  std::cout << "[" << o << "] reclaim object: `" << o->stringify() << "`" << std::endl;
#endif

  delete o;
}

void VM::reset() {
}
void VM::runtime_error(const char* format, ...) {
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

bool VM::call(ClosureObject* closure, sz_t argc) { return false; }
bool VM::call(const Value& callee, sz_t argc) { return false; }
UpvalueObject* VM::capture_upvalue(Value* local) { return nullptr; }
void VM::close_upvalues(Value* last) {}

InterpretRet VM::run() {
  return InterpretRet::OK;
}

}
