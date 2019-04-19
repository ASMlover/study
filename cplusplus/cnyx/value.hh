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

#include "common.hh"

namespace nyx {

enum class ValueType {
  OBJ,
};

class Value : private Copyable {
  using NativeFunction = std::function<Value (int argc, Value* args)>;

  ValueType type_{ValueType::OBJ};
  union {
    BaseObject* obj;
  } as_{};
public:
  Value(void) noexcept {
  }

  Value(ValueType t, BaseObject* o = nullptr) noexcept
    : type_{t}, as_{o} {
  }

  Value(Value&& v) noexcept
    : type_(std::move(v.type_)) {
    as_.obj = std::move(v.as_.obj);
  }

  Value& operator=(const Value& v) noexcept {
    if (this != &v) {
      type_ = v.type_;
      as_.obj = v.as_.obj;
    }
    return *this;
  }

  Value& operator=(Value&& v) noexcept {
    if (this != &v) {
      type_ = std::move(v.type_);
      as_.obj = std::move(v.as_.obj);
    }
    return *this;
  }

  inline BaseObject* as_obj(void) const noexcept {
    return as_.obj;
  }

  bool is_obj(void) const noexcept;
  bool is_nil(void) const noexcept;
  bool is_boolean(void) const noexcept;
  bool is_numeric(void) const noexcept;
  bool is_string(void) const noexcept;
  bool is_closure(void) const noexcept;
  bool is_function(void) const noexcept;
  bool is_native(void) const noexcept;
  bool is_upvalue(void) const noexcept;
  bool is_class(void) const noexcept;
  bool is_instance(void) const noexcept;
  bool is_bound_method(void) const noexcept;

  bool as_boolean(void) const noexcept;
  double as_numeric(void) const noexcept;
  StringObject* as_string(void) const noexcept;
  const char* as_cstring(void) const noexcept;
  ClosureObject* as_closure(void) const noexcept;
  FunctionObject* as_function(void) const noexcept;
  NativeFunction as_native(void) const noexcept;
  ClassObject* as_class(void) const noexcept;
  InstanceObject* as_instance(void) const noexcept;
  BoundMethodObject* as_bound_method(void) const noexcept;

  static Value to_value(BaseObject* obj) { return Value(ValueType::OBJ, obj); }
  static Value make_nil(void) { return Value{}; }
  static Value make_obj(BaseObject* obj) { return Value{ValueType::OBJ, obj}; }
};

using table_t = std::unordered_map<str_t, Value>;

}
