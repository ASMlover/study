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

#include <functional>
#include "common.hh"
#include "object.hh"

namespace tadpole {

enum class ValueType : u8_t {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT,
};

class Value final : public Copyable {
  ValueType type_{ValueType::NIL};
  union {
    bool boolean;
    double numeric;
    BaseObject* object{};
  } as_;

  template <typename T> inline void set_numeric(T x) noexcept { as_.numeric = as_type<double>(x); }
  inline bool is(ObjType type) const noexcept { return is_object() && objtype() == type; }
public:
  Value() noexcept {}
  Value(nil_t) noexcept {}
  Value(bool b) noexcept : type_(ValueType::BOOLEAN) { as_.boolean = b; }
  Value(i8_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u8_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i16_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u16_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i32_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u32_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i64_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u64_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(float n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(double d) noexcept : type_(ValueType::NUMERIC) { as_.numeric = d; }
#if defined(__GNUC__) || defined(__clang__)
  Value(long long n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(unsigned long long n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
#endif
  Value(BaseObject* o) noexcept : type_(ValueType::OBJECT) { as_.object = o; }

  inline ObjType objtype() const noexcept { return as_.object->type(); }

  inline bool is_nil() const noexcept { return type_ == ValueType::NIL; }
  inline bool is_boolean() const noexcept { return type_ == ValueType::BOOLEAN; }
  inline bool is_numeric() const noexcept { return type_ == ValueType::NUMERIC; }
  inline bool is_object() const noexcept { return type_ == ValueType::OBJECT; }
  inline bool is_string() const noexcept { return is(ObjType::STRING); }
  inline bool is_native() const noexcept { return is(ObjType::NATIVE); }
  inline bool is_function() const noexcept { return is(ObjType::FUNCTION); }
  inline bool is_upvalue() const noexcept { return is(ObjType::UPVALUE); }
  inline bool is_closure() const noexcept { return is(ObjType::CLOSURE); }

  inline bool as_boolean() const noexcept { return as_.boolean; }
  inline double as_numeric() const noexcept { return as_.numeric; }
  inline BaseObject* as_object() const noexcept { return as_.object; }
  inline StringObject* as_string() const noexcept { return as_.object->as_string(); }
  inline const char* as_cstring() const noexcept { return as_.object->as_cstring(); }
  inline NativeObject* as_native() const noexcept { return as_.object->as_native(); }
  inline FunctionObject* as_function() const noexcept { return as_.object->as_function(); }
  inline UpvalueObject* as_upvalue() const noexcept { return as_.object->as_upvalue(); }
  inline ClosureObject* as_closure() const noexcept { return as_.object->as_closure(); }

  bool is_truthy() const;
  str_t stringify() const;
};

using TadpoleCFun = std::function<Value (sz_t nargs, Value* args)>;

inline std::ostream& operator<<(std::ostream& out, const Value& val) {
  return out << val.stringify();
}

}
