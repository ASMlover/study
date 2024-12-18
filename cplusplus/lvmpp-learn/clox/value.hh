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

#include <functional>
#include "common.hh"
#include "object_helper.hh"

namespace clox {

enum class ValueType : u8_t {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
};

class Value final : public Copyable {
  ValueType type_{ValueType::VAL_NIL};
  union {
    bool boolean;
    double number;
    Obj* obj{};
  } as_;

  template <typename T> inline void set_number(T x) noexcept { as_.number = as_type<double>(x); }

  bool is_obj_type(ObjType type) const noexcept;
public:
  Value() noexcept {}
  Value(bool b) noexcept : type_{ValueType::VAL_BOOL} { as_.boolean = b; }
  Value(nil_t) noexcept {}
  Value(i8_t n) noexcept : type_{ValueType::VAL_NUMBER} { set_number(n); }
  Value(u8_t n) noexcept : type_{ValueType::VAL_NUMBER} { set_number(n); }
  Value(i16_t n) noexcept : type_{ValueType::VAL_NUMBER} { set_number(n); }
  Value(u16_t n) noexcept : type_{ValueType::VAL_NUMBER} { set_number(n); }
  Value(i32_t n) noexcept : type_{ValueType::VAL_NUMBER} { set_number(n); }
  Value(u32_t n) noexcept : type_{ValueType::VAL_NUMBER} { set_number(n); }
  Value(i64_t n) noexcept : type_{ValueType::VAL_NUMBER} { set_number(n); }
  Value(u64_t n) noexcept : type_{ValueType::VAL_NUMBER} { set_number(n); }
  Value(float f) noexcept : type_{ValueType::VAL_NUMBER} { set_number(f); }
  Value(double d) noexcept : type_{ValueType::VAL_NUMBER} { as_.number = d; }
  Value(Obj* o) noexcept : type_{ValueType::VAL_OBJ} { as_.obj = o; }

  Value(const Value& r) noexcept
    : type_{r.type_} {
    as_.number = r.as_.number;
  }

  Value(Value&& r) noexcept
    : type_{std::move(r.type_)} {
    as_.number = std::move(r.as_.number);
  }

  inline Value& operator=(const Value& r) noexcept {
    if (this != &r) {
      type_ = r.type_;
      as_.number = r.as_.number;
    }
    return *this;
  }

  inline Value& operator=(Value&& r) noexcept {
    if (this != &r) {
      type_ = std::move(r.type_);
      as_.number = std::move(r.as_.number);
    }
    return *this;
  }

  inline bool operator==(const Value& x) const noexcept { return is_equal(x); }
  inline bool operator!=(const Value& x) const noexcept { return !is_equal(x ); }

  inline bool operator<(const Value& r) const noexcept { return as_.number < r.as_.number; }
  inline bool operator<=(const Value& r) const noexcept { return as_.number <= r.as_.number; }
  inline bool operator>(const Value& r) const noexcept { return as_.number > r.as_.number; }
  inline bool operator>=(const Value& r) const noexcept { return as_.number >= r.as_.number; }

  inline bool is_boolean() const noexcept { return type_ == ValueType::VAL_BOOL; }
  inline bool is_nil() const noexcept { return type_ == ValueType::VAL_NIL; }
  inline bool is_number() const noexcept { return type_ == ValueType::VAL_NUMBER; }
  inline bool is_obj() const noexcept { return type_ == ValueType::VAL_OBJ; }
  inline bool is_string() const noexcept { return is_obj_type(ObjType::OBJ_STRING); }
  inline bool is_function() const noexcept { return is_obj_type(ObjType::OBJ_FUNCTION); }
  inline bool is_native() const noexcept { return is_obj_type(ObjType::OBJ_NATIVE); }
  inline bool is_closure() const noexcept { return is_obj_type(ObjType::OBJ_CLOSURE); }
  inline bool is_upvalue() const noexcept { return is_obj_type(ObjType::OBJ_UPVALUE); }

  inline bool as_boolean() const noexcept { return as_.boolean; }
  inline double as_number() const noexcept { return as_.number; }
  inline Obj* as_obj() const noexcept { return as_.obj; }

  ObjString* as_string() const noexcept;
  cstr_t as_cstring() const noexcept;
  ObjFunction* as_function() const noexcept;
  ObjNative* as_native() const noexcept;
  ObjClosure* as_closure() const noexcept;
  ObjUpvalue* as_upvalue() const noexcept;

  inline bool is_falsey() const noexcept {
    return is_nil() || (is_boolean() && !as_boolean());
  }

  bool is_equal(const Value& x) const noexcept;
  str_t stringify() const;
};

inline std::ostream& operator<<(std::ostream& out, const Value& val) noexcept {
  return out << val.stringify();
}

}
