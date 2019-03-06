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
#include <memory>
#include <string>
#include <variant>
#include "common.hh"

namespace sage {

struct Callable;
class Instance;

using CallablePtr = std::shared_ptr<Callable>;
using InstancePtr = std::shared_ptr<Instance>;

class Value : public Copyable {
  using VariantType = std::variant<
    std::nullptr_t,
    bool,
    std::int64_t,
    double,
    std::string,
    CallablePtr,
    InstancePtr>;

  VariantType v_{};

  template <typename T> std::int64_t integer_cast(T x) noexcept {
    return static_cast<std::int64_t>(x);
  }

  template <typename T> double decimal_cast(T x) noexcept {
    return static_cast<double>(x);
  }

  template <typename T> Value& set_integer(T x) noexcept {
    return v_ = static_cast<std::int64_t>(x), *this;
  }

  template <typename T> Value& set_decimal(T x) noexcept {
    return v_ = static_cast<double>(x), *this;
  }

  Value& set_string(const std::string& s) noexcept {
    return v_ = s, *this;
  }

  template <typename Function>
  Value binary_numeric(Function&& fn, const Value& r) const {
    if (is_integer() && r.is_integer())
      return fn(to_integer(), r.to_integer());
    else if (is_integer() && r.is_decimal())
      return fn(to_integer(), r.to_decimal());
    else if (is_decimal() && r.is_integer())
      return fn(to_decimal(), r.to_integer());
    else
      return fn(to_decimal(), r.to_decimal());
  }

  Value binary_add(const Value& r) const {
    // only support string concat and numeric add

    if (is_string() && r.is_string())
      return to_string() + r.to_string();
    else
      return binary_numeric([](auto x, auto y) -> Value { return x + y; }, r);
  }

  Value binary_mod(const Value& r) const {
    // only support integer modulo

    return to_integer() % r.to_integer();
  }

  template <typename Function>
  bool compare_numeric(Function&& fn, const Value& r) const {
    if (is_integer() && r.is_integer())
      return fn(to_integer(), r.to_integer());
    else if (is_integer() && r.is_decimal())
      return fn(to_integer(), r.to_decimal());
    else if (is_decimal() && r.is_integer())
      return fn(to_decimal(), r.to_integer());
    else
      return fn(to_decimal(), r.to_decimal());
  }

  bool compare_equal(const Value& r) const {
    if (is_numeric() && r.is_numeric())
      return compare_numeric([](auto x, auto y) -> bool { return x == y; }, r);
    else
      return v_ == r.v_;
  }
public:
  Value(const Value& r) noexcept : v_(r.v_) {}
  Value(Value&& r) noexcept : v_(std::move(r.v_)) {}
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
#if defined(__GNUC__)
  Value(long long ll) noexcept : v_(integer_cast(ll)) {}
  Value(unsigned long long ull) noexcept : v_(integer_cast(ull)) {}
#endif
  Value(float f) noexcept : v_(decimal_cast(f)) {}
  Value(double d) noexcept : v_(d) {}
  Value(const char* s) noexcept : v_(std::string(s)) {}
  Value(const std::string& s) noexcept : v_(s) {}
  Value(const CallablePtr& c) noexcept : v_(c) {}
  Value(const InstancePtr& i) noexcept : v_(i) {}

  bool is_nil(void) const { return std::holds_alternative<std::nullptr_t>(v_); }
  bool is_boolean(void) const { return std::holds_alternative<bool>(v_); }
  bool is_integer(void) const { return std::holds_alternative<std::int64_t>(v_); }
  bool is_decimal(void) const { return std::holds_alternative<double>(v_); }
  bool is_string(void) const { return std::holds_alternative<std::string>(v_); }
  bool is_callable(void) const { return std::holds_alternative<CallablePtr>(v_); }
  bool is_instance(void) const { return std::holds_alternative<InstancePtr>(v_); }

  bool is_numeric(void) const { return is_integer() || is_decimal(); }

  bool to_boolean(void) const { return std::get<bool>(v_); }
  std::int64_t to_integer(void) const { return std::get<std::int64_t>(v_); }
  double to_decimal(void) const { return std::get<double>(v_); }
  std::string to_string(void) const { return std::get<std::string>(v_); }
  CallablePtr to_callable(void) const { return std::get<CallablePtr>(v_); }
  InstancePtr to_instance(void) const { return std::get<InstancePtr>(v_); }

  operator bool(void) const { return to_boolean(); }
  operator std::int64_t(void) const { return to_integer(); }
  operator double(void) const { return to_decimal(); }
  operator std::string(void) const { return to_string(); }
  operator CallablePtr(void) const { return to_callable(); }
  operator InstancePtr(void) const { return to_instance(); }

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
#if defined(__GNUC__)
  Value& operator=(long long ll) noexcept { return set_integer(ll); }
  Value& operator=(unsigned long long ull) noexcept { return set_integer(ull); }
#endif
  Value& operator=(float f) noexcept { return set_decimal(f); }
  Value& operator=(double d) noexcept { return v_ = d, *this; }
  Value& operator=(const char* s) noexcept { return set_string(s); }
  Value& operator=(const std::string& s) noexcept { return v_ = s, *this; }
  Value& operator=(const CallablePtr& c) noexcept { return v_ = c, *this; }
  Value& operator=(const InstancePtr& i) noexcept { return v_ = i, *this; }

  bool operator==(const Value& r) const { return compare_equal(r); }
  bool operator!=(const Value& r) const { return !compare_equal(r); }
  bool operator<(const Value& r) const { return compare_numeric([](auto x, auto y) -> bool { return x < y; }, r); }
  bool operator<=(const Value& r) const { return compare_numeric([](auto x, auto y) -> bool { return x <= y; }, r); }
  bool operator>(const Value& r) const { return compare_numeric([](auto x, auto y) -> bool { return x > y; }, r); }
  bool operator>=(const Value& r) const { return compare_numeric([](auto x, auto y) -> bool { return x >= y; }, r); }

  Value operator+(const Value& r) const { return binary_add(r); }
  Value operator-(const Value& r) const { return binary_numeric([](auto x, auto y) -> Value { return x - y; }, r); }
  Value operator*(const Value& r) const { return binary_numeric([](auto x, auto y) -> Value { return x * y; }, r); }
  Value operator/(const Value& r) const { return binary_numeric([](auto x, auto y) -> Value { return x / y; }, r); }
  Value operator%(const Value& r) const { return binary_mod(r); }
  Value& operator+=(const Value& r) { return *this = *this + r, *this; }
  Value& operator-=(const Value& r) { return *this = *this - r, *this; }
  Value& operator*=(const Value& r) { return *this = *this * r, *this; }
  Value& operator/=(const Value& r) { return *this = *this / r, *this; }
  Value& operator%=(const Value& r) { return *this = *this % r, *this; }
  Value operator-(void) const { return negative_numeric(); }
  Value operator!(void) const { return !is_truthy(); }

  Value negative_numeric(void) const {
    if (is_integer())
      return -to_integer();
    else
      return -to_decimal();
  }

  bool is_abs_equal(const Value& r) const { return v_ == r.v_; }
  bool is_equal(const Value& r) const { return (this == &r) || (*this == r); }

  bool is_truthy(void) const;
  std::string stringify(void) const;
  std::string type(void) const;
};

std::ostream& operator<<(std::ostream& out, const Value& val);

}
