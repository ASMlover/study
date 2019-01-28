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

#include <cstdint>
#include <ostream>
#include "common.h"
#include "object.h"

namespace lox {

enum class ValueType {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT
};

class Value : public Copyable {
  ValueType type_;
  union {
    bool boolean;
    double numeric;
    Object* object;
  } as_;

  template <typename T> inline double decimal_cast(T x) noexcept {
    return static_cast<double>(x);
  }

  template <typename T> inline void set_numeric(T x) noexcept {
    type_ = ValueType::NUMERIC;
    as_.numeric = static_cast<double>(x);
  }

  inline Value add_numeric(const Value& r) const noexcept {
    return as_numeric() + r.as_numeric();
  }

  inline Value sub_numeric(const Value& r) const noexcept {
    return as_numeric() - r.as_numeric();
  }

  inline Value mul_numeric(const Value& r) const noexcept {
    return as_numeric() * r.as_numeric();
  }

  inline Value div_numeric(const Value& r) const noexcept {
    return as_numeric() / r.as_numeric();
  }
public:
  Value(void) noexcept : type_(ValueType::NIL) { as_.numeric = 0; }
  Value(std::nullptr_t) noexcept : type_(ValueType::NIL) { as_.numeric = 0; }
  Value(bool b) noexcept : type_(ValueType::BOOLEAN) { as_.boolean = b; }
  Value(std::int8_t i8) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(i8); }
  Value(std::uint8_t u8) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(u8); }
  Value(std::int16_t i16) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(i16); }
  Value(std::uint16_t u16) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(u16); }
  Value(std::int32_t i32) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(i32); }
  Value(std::uint32_t u32) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(u32); }
  Value(std::int64_t i64) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(i64); }
  Value(std::uint64_t u64) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(u64); }
#if defined(__GNUC__)
  Value(long long ll) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(ll); }
  Value(unsigned long long ull) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(ull); }
#endif
  Value(float f) noexcept : type_(ValueType::NUMERIC) { as_.numeric = decimal_cast(f); }
  Value(double d) noexcept : type_(ValueType::NUMERIC) { as_.numeric = d; }
  Value(Object* o) noexcept : type_(ValueType::OBJECT) { as_.object = o; }
  Value(const Value& v) : type_(v.type_) { as_.numeric = v.as_.numeric; }
  Value(Value&& v) : type_(std::move(v.type_)) { as_.numeric = std::move(v.as_.numeric); }

  Value& operator=(std::nullptr_t) noexcept { return type_ = ValueType::NIL, as_.numeric = 0, *this; }
  Value& operator=(bool b) noexcept { return type_ = ValueType::BOOLEAN, as_.boolean = b, *this; }
  Value& operator=(std::int8_t i8) noexcept { return set_numeric(i8), *this; }
  Value& operator=(std::uint8_t u8) noexcept { return set_numeric(u8), *this; }
  Value& operator=(std::int16_t i16) noexcept { return set_numeric(i16), *this; }
  Value& operator=(std::uint16_t u16) noexcept { return set_numeric(u16), *this; }
  Value& operator=(std::int32_t i32) noexcept { return set_numeric(i32), *this; }
  Value& operator=(std::uint32_t u32) noexcept { return set_numeric(u32), *this; }
  Value& operator=(std::int64_t i64) noexcept { return set_numeric(i64), *this; }
  Value& operator=(std::uint64_t u64) noexcept { return set_numeric(u64), *this; }
#if defined(__GNUC__)
  Value& operator=(long long ll) noexcept { return set_numeric(ll), *this; }
  Value& operator=(unsigned long long ull) noexcept { return set_numeric(ull), *this; }
#endif
  Value& operator=(float f) noexcept { return set_numeric(f), *this; }
  Value& operator=(double d) noexcept { return type_ = ValueType::NUMERIC, as_.numeric = d, *this; }
  Value& operator=(Object* o) noexcept { return type_ = ValueType::OBJECT, as_.object = o, *this; }

  Value& operator=(const Value& v) noexcept {
    if (this != &v) {
      type_ = v.type_;

      if (type_ == ValueType::OBJECT)
        as_.object = v.as_.object;
      else
        as_.numeric = v.as_.numeric;
    }
    return *this;
  }

  Value& operator=(Value&& v) noexcept {
    if (this != &v) {
      type_ = std::move(v.type_);

      if (type_ == ValueType::OBJECT)
        as_.object = std::move(v.as_.object);
      else
        as_.numeric = std::move(v.as_.numeric);
    }
    return *this;
  }

  Value operator+(const Value& r) const noexcept { return add_numeric(r); }
  Value operator-(const Value& r) const noexcept { return sub_numeric(r); }
  Value operator*(const Value& r) const noexcept { return mul_numeric(r); }
  Value operator/(const Value& r) const noexcept { return div_numeric(r); }

  bool operator==(const Value& r) const { return is_equal(r); }
  bool operator>(const Value& r) const { return as_numeric() > r.as_numeric(); }
  bool operator>=(const Value& r) const { return as_numeric() >= r.as_numeric(); }
  bool operator<(const Value& r) const { return as_numeric() < r.as_numeric(); }
  bool operator<=(const Value& r) const { return as_numeric() <= r.as_numeric(); }

  bool is_nil(void) const noexcept { return type_ == ValueType::NIL; }
  bool is_boolean(void) const noexcept { return type_ == ValueType::BOOLEAN; }
  bool is_numeric(void) const noexcept { return type_ == ValueType::NUMERIC; }
  bool is_object(void) const noexcept { return type_ == ValueType::OBJECT; }
  bool is_object(ObjType t) const noexcept { return is_object() && as_.object->get_type() == t; }

  bool as_boolean(void) const noexcept { return as_.boolean; }
  double as_numeric(void) const noexcept { return as_.numeric; }
  Object* as_object(void) const noexcept { return as_.object; }

  operator bool(void) const noexcept { return as_boolean(); }
  operator double(void) const noexcept { return as_numeric(); }
  operator Object*(void) const noexcept { return as_object(); }

  Value operator-(void) const noexcept { return -as_numeric(); }

  bool is_equal(const Value& r) const;
  bool is_truthy(void) const;
  std::string stringify(void) const;
};

std::ostream& operator<<(std::ostream& out, const Value& value);

}
