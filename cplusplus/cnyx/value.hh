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

#include <ostream>
#include "common.hh"

namespace nyx {

enum class ValueType {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT,
};

class Value : private Copyable {
  using NativeFunction = std::function<Value (int argc, Value* args)>;

  ValueType type_{ValueType::NIL};
  union {
    bool boolean;
    double numeric;
    BaseObject* obj;
  } as_{};
public:
  Value(void) noexcept
    : type_{ValueType::NIL} {
  }

  Value(nil_t) noexcept
    : type_{ValueType::NIL} {
  }

  Value(bool b) noexcept
    : type_{ValueType::BOOLEAN} {
    as_.boolean = b;
  }

  Value(double d) noexcept
    : type_{ValueType::NUMERIC} {
    as_.numeric = d;
  }

  Value(BaseObject* o) noexcept
    : type_{ValueType::OBJECT} {
    as_.obj = o;
  }

  Value(const Value& v) noexcept
    : type_{v.type_} {
    if (type_ == ValueType::OBJECT)
      as_.obj = v.as_.obj;
    else
      as_.numeric = v.as_.numeric;
  }

  Value(Value&& v) noexcept
    : type_{std::move(v.type_)} {
    if (type_ == ValueType::OBJECT)
      as_.obj = std::move(v.as_.obj);
    else
      as_.numeric = std::move(v.as_.numeric);
  }

  Value& operator=(const Value& v) noexcept {
    if (this != &v) {
      type_ = v.type_;
      if (type_ == ValueType::OBJECT)
        as_.obj = v.as_.obj;
      else
        as_.numeric = v.as_.numeric;
    }
    return *this;
  }

  Value& operator=(Value&& v) noexcept {
    if (this != &v) {
      type_ = std::move(v.type_);
      if (type_ == ValueType::OBJECT)
        as_.obj = std::move(v.as_.obj);
      else
        as_.numeric = std::move(v.as_.numeric);
    }
    return *this;
  }

  inline ValueType type(void) const { return type_; }
  inline bool is_nil(void) const noexcept { return type_ == ValueType::NIL; }
  inline bool is_boolean(void) const noexcept { return type_ == ValueType::BOOLEAN; }
  inline bool is_numeric(void) const noexcept { return type_ == ValueType::NUMERIC; }
  inline bool is_object(void) const noexcept { return type_ == ValueType::OBJECT; }

  inline bool as_boolean(void) const noexcept { return as_.boolean; }
  inline double as_numeric(void) const noexcept { return as_.numeric; }
  inline BaseObject* as_object(void) const noexcept { return as_.obj; }

  bool is_falsely(void) const { return is_nil() || (is_boolean() && !as_boolean()); }

  bool is_string(void) const noexcept;
  bool is_closure(void) const noexcept;
  bool is_function(void) const noexcept;
  bool is_native(void) const noexcept;
  bool is_upvalue(void) const noexcept;
  bool is_class(void) const noexcept;
  bool is_instance(void) const noexcept;
  bool is_bound_method(void) const noexcept;

  StringObject* as_string(void) const noexcept;
  const char* as_cstring(void) const noexcept;
  ClosureObject* as_closure(void) const noexcept;
  FunctionObject* as_function(void) const noexcept;
  NativeFunction as_native(void) const noexcept;
  ClassObject* as_class(void) const noexcept;
  InstanceObject* as_instance(void) const noexcept;
  BoundMethodObject* as_bound_method(void) const noexcept;

  str_t stringify(void) const;

  static Value to_value(BaseObject* obj) { return Value{obj}; }
  static Value make_nil(void) { return Value{nullptr}; }
  static Value make_boolean(bool b) { return Value{b}; }
  static Value make_numeric(double d) { return Value{d}; }
  static Value make_object(BaseObject* obj) { return Value{obj}; }
};

bool operator==(const Value& a, const Value& b);
bool operator!=(const Value& a, const Value& b);
std::ostream& operator<<(std::ostream& out, const Value& v);

using table_t = std::unordered_map<str_t, Value>;

}
