// Copyright (c) 2023 ASMlover. All rights reserved.
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
#pragma once

#include <unordered_map>
#include <list>
#include "common.hh"
#include "value.hh"
#include "object.hh"

namespace clox {

enum class InterpretResult : u8_t {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
};

struct CallFrame {
  ObjClosure* closure;
  u8_t* ip;
  Value* slots;

  inline void set_callframe(ObjClosure* arg_closure, u8_t* arg_ip, Value* arg_slots) noexcept {
    closure = arg_closure;
    ip = arg_ip;
    slots = arg_slots;
  }
};

class GlobalCompiler;

class VM final : private UnCopyable {
  static constexpr sz_t kFramesMax = 64;
  static constexpr sz_t kStackMax = kFramesMax * 256;
  static constexpr sz_t kGCThreshold = 1 << 3;

  GlobalCompiler* gcompiler_{};
  const u8_t* ip_;
  Value stack_[kStackMax];
  Value* stack_top_;

  CallFrame frames_[kFramesMax];
  int frame_count_{};

  std::unordered_map<str_t, Value> globals_;
  std::unordered_map<u32_t, ObjString*> strings_;
  ObjUpvalue* open_upvalues_{};
  std::list<Obj*> objects_;
  sz_t gc_threshold_{kGCThreshold};
  std::vector<Obj*> gray_stack_;

  inline void reset_stack() noexcept {
    stack_top_ = stack_;
    frame_count_ = 0;
    open_upvalues_ = nullptr;
  }

  inline void push(const Value& value) noexcept { *stack_top_++ = value; }
  inline Value pop() noexcept { return *(--stack_top_); }

  inline Value peek(int distance = 0) const noexcept {
    return stack_top_[-1 - distance];
  }

  bool call(ObjClosure* closure, int arg_count) noexcept;
  bool call_value(const Value& callee, int arg_count) noexcept;

  ObjUpvalue* capture_upvalue(Value* local) noexcept;
  void close_upvalues(Value* last) noexcept;

  void runtime_error(const char* format, ...) noexcept;
  void define_native(const str_t& name, NativeFn&& function) noexcept;

  void mark_roots() noexcept;
  void free_object(Obj* o) noexcept;

  InterpretResult run() noexcept;
public:
  VM() noexcept;
  ~VM() noexcept;

  InterpretResult interpret(const str_t& source) noexcept;

  void append_object(Obj* o) noexcept;
  void collect_garbage() noexcept;
  void mark_object(Obj* object) noexcept;
  void mark_value(Value& value) noexcept;
  void free_objects() noexcept;

  inline void set_interned(u32_t hash, ObjString* str) noexcept {
    strings_[hash] = str;
  }

  inline ObjString* get_interned(u32_t hash) const noexcept {
    if (auto it = strings_.find(hash); it != strings_.end())
      return it->second;
    return nullptr;
  }
};

void init_vm() noexcept;
void free_vm() noexcept;
VM& get_vm() noexcept;

}
