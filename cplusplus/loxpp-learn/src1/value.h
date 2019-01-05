// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <string>
#include <variant>
#include "common.h"

namespace lox {

class Value : private Copyable {
  using VariantType = std::variant<
    std::nullptr_t,
    bool,
    std::int64_t,
    double,
    std::string>;

  VariantType v_{};

  template <typename T> std::int64_t integer_cast(T x) noexcept {
    return static_cast<std::int64_t>(x);
  }

  template <typename T> Value& set_integer(T x) noexcept {
    return v_ = static_cast<std::int64_t>(x), *this;
  }

  Value& set_string(const std::string& s) noexcept {
    return v_ = s, *this;
  }

  Value add_operation(const Value& r) const {
    if (is_string() && r.is_string())
      return to_string() + r.to_string();
    else if (is_numeric() && r.is_numeric())
      return binary_numeric(add(), r);
    return nullptr;
  }

  Value add_string(const std::string& s) const {
    return to_string() + s;
  }

  struct add {
    template <typename X, typename Y>
    Value operator()(X x, Y y) const { return x + y; }
  };

  struct sub {
    template <typename X, typename Y>
    Value operator()(X x, Y y) const { return x - y; }
  };

  struct mul {
    template <typename X, typename Y>
    Value operator()(X x, Y y) const { return x * y; }
  };

  struct div {
    template <typename X, typename Y>
    Value operator()(X x, Y y) const { return x / y; }
  };

  template <typename BinOp>
  Value binary_numeric(BinOp&& op, const Value& r) const {
    if (is_integer() && r.is_integer())
      return op(to_integer(), r.to_integer());
    else if (is_integer() && r.is_decimal())
      return op(to_integer(), r.to_decimal());
    else if (is_decimal() && r.is_integer())
      return op(to_decimal(), r.to_integer());
    else
      return op(to_decimal(), r.to_decimal());
  }

  Value mod_numeric(const Value& r) const {
    return to_integer() % r.to_integer();
  }
public:
  Value(void) noexcept : v_(nullptr) {}
  Value(std::nullptr_t) noexcept : v_(nullptr) {}
  Value(bool b) noexcept : v_(b) {}
  Value(std::int8_t i8) noexcept : v_(integer_cast(i8)) {}
  Value(std::uint8_t u8) noexcept : v_(integer_cast(u8)) {}
  Value(std::int16_t i16) noexcept : v_(integer_cast(i16)) {}
  Value(std::uint16_t u16) noexcept : v_(integer_cast(u16)) {}
  Value(std::int32_t i32) noexcept : v_(integer_cast(i32)) {}
  Value(std::uint32_t u32) noexcept : v_(integer_cast(u32)) {}
  Value(std::int64_t i64) noexcept : v_(i64) {}
  Value(std::uint64_t u64) noexcept : v_(integer_cast(u64)) {}
  Value(double d) noexcept : v_(d) {}
  Value(const char* s) noexcept : v_(std::string(s)) {}
  Value(const std::string& s) noexcept : v_(s) {}
  Value(const Value& r) noexcept : v_(r.v_) {}
  Value(Value&& r) noexcept : v_(std::move(r.v_)) {}

  bool is_nil(void) const { return std::holds_alternative<std::nullptr_t>(v_); }
  bool is_boolean(void) const { return std::holds_alternative<bool>(v_); }
  bool is_integer(void) const { return std::holds_alternative<std::int64_t>(v_); }
  bool is_decimal(void) const { return std::holds_alternative<double>(v_); }
  bool is_string(void) const { return std::holds_alternative<std::string>(v_); }

  bool is_numeric(void) const { return is_integer() || is_decimal(); }

  bool to_boolean(void) const { return std::get<bool>(v_); }
  std::int64_t to_integer(void) const { return std::get<std::int64_t>(v_); }
  double to_decimal(void) const { return std::get<double>(v_); }
  std::string to_string(void) const { return std::get<std::string>(v_); }

  operator bool(void) const { return to_boolean(); }
  operator std::int64_t(void) const { return to_integer(); }
  operator double(void) const { return to_decimal(); }
  operator std::string(void) const { return to_string(); }

  bool operator==(const Value& r) const { return v_ == r.v_; }
  bool operator!=(const Value& r) const { return v_ != r.v_; }
  bool operator<(const Value& r) const { return v_ < r.v_; }
  bool operator<=(const Value& r) const { return v_ <= r.v_; }
  bool operator>(const Value& r) const { return v_ > r.v_; }
  bool operator>=(const Value& r) const { return v_ >= r.v_; }

  Value operator+(const Value& r) const { return add_operation(r); }
  Value operator-(const Value& r) const { return binary_numeric(sub(), r); }
  Value operator*(const Value& r) const { return binary_numeric(mul(), r); }
  Value operator/(const Value& r) const { return binary_numeric(div(), r); }
  Value operator%(const Value& r) const { return mod_numeric(r); }
  Value& operator+=(const Value& r) { return *this = *this + r, *this; }
  Value& operator-=(const Value& r) { return *this = *this - r, *this; }
  Value& operator*=(const Value& r) { return *this = *this * r, *this; }
  Value& operator/=(const Value& r) { return *this = *this / r, *this; }
  Value& operator%=(const Value& r) { return *this = *this % r, *this; }

  Value& operator=(const Value& r) noexcept { return v_ = r.v_, *this; }
  Value& operator=(Value&& r) noexcept { return v_ = std::move(r.v_), *this; }
  Value& operator=(std::nullptr_t) noexcept { return v_ = nullptr, *this; }
  Value& operator=(bool b) noexcept { return v_ = b, *this; }
  Value& operator=(std::int8_t i8) noexcept { return set_integer(i8); }
  Value& operator=(std::uint8_t u8) noexcept { return set_integer(u8); }
  Value& operator=(std::int16_t i16) noexcept { return set_integer(i16); }
  Value& operator=(std::uint16_t u16) noexcept { return set_integer(u16); }
  Value& operator=(std::int32_t i32) noexcept { return set_integer(i32); }
  Value& operator=(std::uint32_t u32) noexcept { return set_integer(u32); }
  Value& operator=(std::int64_t i64) noexcept { return set_integer(i64); }
  Value& operator=(std::uint64_t u64) noexcept { return set_integer(u64); }
  Value& operator=(double d) noexcept { return v_ = d, *this; }
  Value& operator=(const char* s) noexcept { return set_string(s); }
  Value& operator=(const std::string& s) noexcept { return set_string(s); }

  bool is_truthy(void) const;
  std::string stringify(void) const;
  std::string type(void) const;
};

std::ostream& operator<<(std::ostream& out, const Value& r);

}
