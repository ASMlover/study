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

namespace clox {

enum class InterpretResult : u8_t {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
};

class Chunk;

class VM final : private UnCopyable {
  static constexpr sz_t kStackMax = 256;

  Chunk* chunk_;
  const u8_t* ip_;
  Value stack_[kStackMax];
  Value* stack_top_;

  std::unordered_map<u32_t, ObjString*> strings_;
  std::list<Obj*> objects_;

  inline void reset_stack() noexcept { stack_top_ = stack_; }
  inline void push(const Value& value) noexcept { *stack_top_++ = value; }
  inline Value pop() noexcept { return *(--stack_top_); }

  inline Value peek(int distance = 0) const noexcept {
    return stack_top_[-1 - distance];
  }

  void runtime_error(const char* format, ...) noexcept;
  void free_object(Obj* o) noexcept;

  InterpretResult run() noexcept;
public:
  VM() noexcept;
  ~VM() noexcept;

  InterpretResult interpret(const str_t& source) noexcept;

  void append_object(Obj* o) noexcept;
  void free_objects() noexcept;

  inline Chunk* get_chunk() const noexcept { return chunk_; }

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
