// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include <Object/Object.hh>

namespace Tadpole::Value {

enum class ValueType : u8_t {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT,
};

class Value final : public Common::Copyable {
  ValueType type_{ValueType::NIL};
  union {
    bool boolean;
    double numeric;
    Object::BaseObject* object{};
  } as_;

  template <typename T> inline void set_numeric(T x) noexcept { as_.numeric = Common::as_type<double>(x); }
  inline bool is(Object::ObjType type) const noexcept { return is_object() && objtype() == type; }
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
#if defined(TADPOLE_GNUC)
  Value(long long n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(unsigned long long n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
#endif
  Value(Object::BaseObject* o) noexcept : type_(ValueType::OBJECT) { as_.object = o; }

  inline Object::ObjType objtype() const noexcept { return as_.object->type(); }

  inline bool is_nil() const noexcept { return type_ == ValueType::NIL; }
  inline bool is_boolean() const noexcept { return type_ == ValueType::BOOLEAN; }
  inline bool is_numeric() const noexcept { return type_ == ValueType::NUMERIC; }
  inline bool is_object() const noexcept { return type_ == ValueType::OBJECT; }
  inline bool is_string() const noexcept { return is(Object::ObjType::STRING); }
  inline bool is_native() const noexcept { return is(Object::ObjType::NATIVE); }
  inline bool is_function() const noexcept { return is(Object::ObjType::FUNCTION); }
  inline bool is_upvalue() const noexcept { return is(Object::ObjType::UPVALUE); }
  inline bool is_closure() const noexcept { return is(Object::ObjType::CLOSURE); }

  inline bool as_boolean() const noexcept { return as_.boolean; }
  inline double as_numeric() const noexcept { return as_.numeric; }
  template <typename T> inline T as_integer() const noexcept { return Common::as_type<T>(as_.numeric); }
  inline Object::BaseObject* as_object() const noexcept { return as_.object; }
  inline Object::BaseObject* as_object(safe_t) const noexcept { return is_object() ? as_.object : nullptr; }
  inline Object::StringObject* as_string() const noexcept { return as_.object->as_string(); }
  inline const char* as_cstring() const noexcept { return as_.object->as_cstring(); }
  inline Object::NativeObject* as_native() const noexcept { return as_.object->as_native(); }
  inline Object::FunctionObject* as_function() const noexcept { return as_.object->as_function(); }
  inline Object::UpvalueObject* as_upvalue() const noexcept { return as_.object->as_upvalue(); }
  inline Object::ClosureObject* as_closure() const noexcept { return as_.object->as_closure(); }

  bool is_equal_as_string(Object::StringObject* s) const noexcept;
  bool is_equal_as_string(const str_t& s) const noexcept;
  bool is_equal_as_string(strv_t s) const noexcept;
  bool is_equal_as_string(const char* s) const noexcept;

  bool is_truthy() const;
  str_t stringify() const;
};

using TadpoleCFun = std::function<Value (sz_t nargs, Value* args)>;

inline std::ostream& operator<<(std::ostream& out, const Value& val) noexcept {
  return out << val.stringify();
}

}
