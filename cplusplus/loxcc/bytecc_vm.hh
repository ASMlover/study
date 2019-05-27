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
#pragma once

#include <memory>
#include <list>
#include <vector>
#include <unordered_map>
#include "common.hh"
#include "bytecc_value.hh"

namespace loxcc::bytecc {

enum class InterpretRet {
  OK,
  COMPILE_ERR,
  RUNTIME_ERR,
};

class CallFrame;
class GlobalCompiler;

class VM final : private UnCopyable {
  static constexpr sz_t kMaxFrames = 256;
  static constexpr sz_t kHeapGrowFactor = 2;
  static constexpr sz_t kGCThresholds = 1 << 20;

  std::unique_ptr<GlobalCompiler> gcompiler_;

  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;

  std::unordered_map<str_t, Value> globals_;
  std::unordered_map<u32_t, StringObject*> interned_strings_;

  StringObject* ctor_string_{};
  UpvalueObject* open_upvalues_{};

  sz_t bytes_allocated_{};
  sz_t next_gc_{kGCThresholds};
  std::list<BaseObject*> all_objects_;
  std::list<BaseObject*> worked_objects_;

  void runtime_error(const char* format, ...);
  void reset(void);
  Value* stack_values(int distance);
  void stack_resize(int distance);
  void set_stack(int distance, const Value& v);

  void push(const Value& value);
  Value pop(void);
  const Value& peek(int distance = 0) const;

  void define_native(const str_t& name, const NativeFn& fn);
  void define_native(const str_t& name, NativeFn&& fn);
  void define_method(StringObject* name);
  bool bind_method(ClassObject* cls, StringObject* name);
  bool call(ClosureObject* closure, int argc);
  bool call_value(const Value& callee, int argc);
  bool invoke_from_class(ClassObject* cls, StringObject* name, int argc);
  bool invoke(StringObject* name, int argc);

  UpvalueObject* capture_upvalue(Value* local);
  void close_upvalues(Value* last);

  InterpretRet run(void);

  void collect(void);
public:
  VM(void) noexcept;
  ~VM(void);

  inline void invoke_push(const Value& v) { push(v); }
  inline Value invoke_pop(void) { return pop(); }

  void set_interned(u32_t h, StringObject* s) {
    interned_strings_[h] = s;
  }

  StringObject* get_interned(u32_t h) const {
    if (auto it = interned_strings_.find(h); it != interned_strings_.end())
      return it->second;
    return nullptr;
  }

  void append_object(BaseObject* o);
  void mark_object(BaseObject* o);
  void mark_value(const Value& v);
  void free_object(BaseObject* o);

  InterpretRet interpret(const str_t& source_bytes);
};

}
