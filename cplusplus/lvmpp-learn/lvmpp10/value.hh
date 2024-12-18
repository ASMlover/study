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
#include "common.hh"

namespace lvm {

enum class ValueType {
  NIL,
  BOOLEAN,
  NUMERIC,
};

class Value : public Copyable {
  ValueType type_{ValueType::NIL};
  union {
    bool boolean;
    double numeric;
  } as_;

  template <typename T> inline double decimal_cast(T x) noexcept {
    return static_cast<double>(x);
  }

  template <typename T> inline Value& set_numeric(T x) noexcept {
    type_ = ValueType::NUMERIC;
    as_.numeric = static_cast<double>(x);
    return *this;
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
  Value(void) noexcept { as_.numeric = 0; }
  Value(std::nullptr_t) noexcept { as_.numeric = 0; }
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
  Value(const Value& r) : type_(r.type_) { as_.numeric = r.as_.numeric; }
  Value(Value&& r) : type_(std::move(r.type_)) { as_.numeric = std::move(r.as_.numeric); }

  Value& operator=(std::nullptr_t) noexcept { return type_ = ValueType::NIL, as_.numeric = 0, *this; }
  Value& operator=(bool b) noexcept { return type_ = ValueType::BOOLEAN, as_.boolean = b, *this; }
  Value& operator=(std::int8_t i8) noexcept { return set_numeric(i8); }
  Value& operator=(std::uint8_t u8) noexcept { return set_numeric(u8); }
  Value& operator=(std::int16_t i16) noexcept { return set_numeric(i16); }
  Value& operator=(std::uint16_t u16) noexcept { return set_numeric(u16); }
  Value& operator=(std::int32_t i32) noexcept { return set_numeric(i32); }
  Value& operator=(std::uint32_t u32) noexcept { return set_numeric(u32); }
  Value& operator=(std::int64_t i64) noexcept { return set_numeric(i64); }
  Value& operator=(std::uint64_t u64) noexcept { return set_numeric(u64); }
#if defined(__GNUC__)
  Value& operator=(long long ll) noexcept { return set_numeric(ll); }
  Value& operator=(unsigned long long ull) noexcept { return set_numeric(ull); }
#endif
  Value& operator=(float f) noexcept { return set_numeric(f); }
  Value& operator=(double d) noexcept { return type_ = ValueType::NUMERIC, as_.numeric = d, *this; }

  Value& operator=(const Value& r) noexcept {
    if (this != &r) {
      type_ = r.type_;
      as_.numeric = r.as_.numeric;
    }
    return *this;
  }

  Value& operator=(Value&& r) noexcept {
    if (this != &r) {
      type_ = std::move(r.type_);
      as_.numeric = std::move(r.as_.numeric);
    }
    return *this;
  }

  bool is_nil(void) const noexcept { return type_ == ValueType::NIL; }
  bool is_boolean(void) const noexcept { return type_ == ValueType::BOOLEAN; }
  bool is_numeric(void) const noexcept { return type_ == ValueType::NUMERIC; }

  bool as_boolean(void) const noexcept { return as_.boolean; }
  double as_numeric(void) const noexcept { return as_.numeric; }

  operator bool(void) const noexcept { return as_boolean(); }
  operator double(void) const noexcept { return as_numeric(); }

  Value operator-(void) const noexcept { return -as_numeric(); }
  Value operator!(void) const noexcept { return !is_truthy(); }
  Value operator+(const Value& r) const noexcept { return add_numeric(r); }
  Value operator-(const Value& r) const noexcept { return sub_numeric(r); }
  Value operator*(const Value& r) const noexcept { return mul_numeric(r); }
  Value operator/(const Value& r) const noexcept { return div_numeric(r); }

  bool operator==(const Value& r) const { return is_equal(r); }
  bool operator!=(const Value& r) const { return !is_equal(r); }
  bool operator>(const Value& r) const { return as_numeric() > r.as_numeric(); }
  bool operator>=(const Value& r) const { return as_numeric() >= r.as_numeric(); }
  bool operator<(const Value& r) const { return as_numeric() < r.as_numeric(); }
  bool operator<=(const Value& r) const { return as_numeric() <= r.as_numeric(); }

  bool is_equal(const Value& r) const;
  bool is_truthy(void) const;
  std::string stringify(void) const;
};

std::ostream& operator<<(std::ostream& out, const Value& value);

}
