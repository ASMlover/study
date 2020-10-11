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
#pragma once

#include <list>
#include <vector>
#include <unordered_map>
#include "common.hh"
#include "object.hh"
#include "value.hh"

namespace tadpole {

enum class InterpretRet {
  OK,
  ECOMPILE, // compile error
  ERUNTIME, // runtime error
};

class CallFrame;
class GlobalCompiler;

class VM final : private UnCopyable {
  static constexpr sz_t kGCThreshold  = 1 << 10;
  static constexpr sz_t kGCFactor     = 2;
  static constexpr sz_t kDefaultCap   = 256;

  GlobalCompiler* gcompiler_{};
  bool is_running_{true};

  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;

  std::unordered_map<str_t, Value> globals_;
  std::unordered_map<u32_t, StringObject*> interned_strings_;
  UpvalueObject* open_upvalues_{};

  sz_t gc_threshold_{kGCThreshold};
  std::list<BaseObject*> all_objects_;
  std::list<BaseObject*> worklist_;

  void reset();
  void runtime_error(const char* format, ...);

  void push(Value value) noexcept;
  Value pop() noexcept;
  const Value& peek(sz_t distance = 0) const noexcept;

  bool call(ClosureObject* closure, sz_t argc);
  bool call(const Value& callee, sz_t argc);
  UpvalueObject* capture_upvalue(Value* local);
  void close_upvalues(Value* last);

  InterpretRet run();

  void collect();
  void reclaim_object(BaseObject* o);
public:
  VM() noexcept;
  ~VM();

  void define_native(const str_t& name, TadpoleCFun&& fn);

  void append_object(BaseObject* o);
  void mark_object(BaseObject* o);
  void mark_value(const Value& v);

  InterpretRet interpret(const str_t& source_bytes);

  inline bool is_running() const noexcept { return is_running_; }
  inline void terminate() noexcept { is_running_ = false; }

  inline void set_interned(u32_t h, StringObject* s) noexcept {
    interned_strings_[h] = s;
  }

  inline StringObject* get_interned(u32_t h) const noexcept {
    if (auto it = interned_strings_.find(h); it != interned_strings_.end())
      return it->second;
    return nullptr;
  }
};

}
