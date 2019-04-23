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

#include <list>
#include <optional>
#include <tuple>
#include <vector>
#include "common.hh"
#include "object.hh"

namespace nyx {

enum class InterpretResult {
  OK,
  COMPILE_ERROR,
  RUNTIME_ERROR,
};

class CallFrame;

class VM : private UnCopyable {
  static constexpr sz_t kGCGrowFactor = 2;
  static constexpr sz_t kGCThresholds = (1 << 20);

  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;

  table_t globals_;
  std::unordered_map<u32_t, StringObject*> intern_strings_;
  StringObject* ctor_string_{};

  UpvalueObject* open_upvalues_{};

  sz_t bytes_allocated_{};
  sz_t next_gc_{kGCThresholds};
  std::list<BaseObject*> objects_;
  std::list<BaseObject*> gray_stack_;

  void define_native(const str_t& name, const NativeFunction& fn);
  void define_native(const str_t& name, NativeFunction&& fn);

  void create_class(StringObject* name, ClassObject* superclass = nullptr);
  void define_method(StringObject* name);
  bool bind_method(ClassObject* cls, StringObject* name);

  void reset_stack(void);
  void push(const Value& val);
  Value pop(void);
  Value& peek(int distance = 0);
  const Value& peek(int distance = 0) const;
  void runtime_error(const char* format, ...);

  std::optional<bool> pop_boolean(void);
  std::optional<double> pop_numeric(void);
  std::optional<std::tuple<double, double>> pop_numerics(void);

  void collect(void);
  void remove_undark_intern_strings(void);

  bool call(ClosureObject* closure, int argc);
  bool call_value(const Value& callee, int argc = 0);
  bool invoke_from_class(ClassObject* cls, StringObject* name, int argc);
  bool invoke(StringObject* method, int argc);
  UpvalueObject* capture_upvalue(Value* local);
  void close_upvalues(Value* last);

  bool run(void);
public:
  VM(void);
  ~VM(void);

  inline void invoke_push(const Value& v) { push(v); }
  inline Value invoke_pop(void) { return pop(); }

  void append_object(BaseObject* o);
  void set_intern_string(u32_t hash, StringObject* o);
  std::optional<StringObject*> get_intern_string(u32_t hash) const;

  void gray_object(BaseObject* obj);
  void gray_value(const Value& v);
  void blacken_object(BaseObject* obj);
  void free_object(BaseObject* obj);

  InterpretResult interpret(const str_t& source_bytes);
};

}
