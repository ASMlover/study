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

#include <string>
#include <variant>
#include "common.h"

namespace lox {

class Value : private Copyable {
  using VariantType = std::variant<std::nullptr_t, bool, double, std::string>;

  VariantType v_{};

  template <typename T>
  Value& set_numeric(T r) noexcept {
    return v_ = static_cast<double>(r), *this;
  }

  Value& set_string(const std::string& s) noexcept {
    return v_ = s, *this;
  }

  Value add_operation(const Value& r) const {
    if (is_string() && r.is_string())
      return to_string() + r.to_string();
    else if (is_numeric() && r.is_numeric())
      return to_numeric() + r.to_numeric();
    return Value();
  }

  Value add_string(const std::string& s) const {
    return to_string() + s;
  }

  template <typename T>
  Value add_numeric(T r) const { return to_numeric() + static_cast<double>(r); }
  template <typename T>
  Value sub_numeric(T r) const { return to_numeric() - static_cast<double>(r); }
  template <typename T>
  Value mul_numeric(T r) const { return to_numeric() * static_cast<double>(r); }
  template <typename T>
  Value div_numeric(T r) const { return to_numeric() / static_cast<double>(r); }
public:
  Value(void) noexcept : v_(nullptr) {}
  Value(bool b) noexcept : v_(b) {}
  Value(int i) noexcept : v_(static_cast<double>(i)) {}
  Value(unsigned int ui) noexcept : v_(static_cast<double>(ui)) {}
  Value(long l) noexcept : v_(static_cast<double>(l)) {}
  Value(unsigned long ul) noexcept : v_(static_cast<double>(ul)) {}
  Value(long long ll) noexcept : v_(static_cast<double>(ll)) {}
  Value(unsigned long long ull) noexcept : v_(static_cast<double>(ull)) {}
  Value(double d) noexcept : v_(d) {}
  Value(const char* s) noexcept : v_(std::string(s)) {}
  Value(const std::string& s) noexcept : v_(s) {}
  Value(const Value& r) noexcept : v_(r.v_) {}
  Value(Value&& r) noexcept : v_(std::move(r.v_)) {}

  bool is_nil(void) const { return std::holds_alternative<std::nullptr_t>(v_); }
  bool is_boolean(void) const { return std::holds_alternative<bool>(v_); }
  bool is_numeric(void) const { return std::holds_alternative<double>(v_); }
  bool is_string(void) const { return std::holds_alternative<std::string>(v_); }

  bool to_boolean(void) const { return std::get<bool>(v_); }
  double to_numeric(void) const { return std::get<double>(v_); }
  std::string to_string(void) const { return std::get<std::string>(v_); }

  operator bool(void) const { return to_boolean(); }

  bool operator==(const Value& r) const { return v_ == r.v_; }
  bool operator!=(const Value& r) const { return v_ != r.v_; }
  bool operator<(const Value& r) const { return v_ < r.v_; }
  bool operator<=(const Value& r) const { return v_ <= r.v_; }
  bool operator>(const Value& r) const { return v_ > r.v_; }
  bool operator>=(const Value& r) const { return v_ >= r.v_; }

  Value operator+(const Value& r) const { return add_operation(r); }
  Value operator+(int i) const { return add_numeric(i); }
  Value operator+(unsigned int ui) const { return add_numeric(ui); }
  Value operator+(long l) const { return add_numeric(l); }
  Value operator+(unsigned long ul) const { return add_numeric(ul); }
  Value operator+(long long ll) const { return add_numeric(ll); }
  Value operator+(unsigned long long ull) const { return add_numeric(ull); }
  Value operator+(double d) const { return add_numeric(d); }
  Value operator+(const char* s) const { return add_string(s); }
  Value operator+(const std::string& s) const { return add_string(s); }
  Value operator-(const Value& r) const { return to_numeric() - r.to_numeric(); }
  Value operator-(int i) const { return sub_numeric(i); }
  Value operator-(unsigned int ui) const { return sub_numeric(ui); }
  Value operator-(long l) const { return sub_numeric(l); }
  Value operator-(unsigned long ul) const { return sub_numeric(ul); }
  Value operator-(long long ll) const { return sub_numeric(ll); }
  Value operator-(unsigned long long ull) const { return sub_numeric(ull); }
  Value operator-(double d) const { return sub_numeric(d); }
  Value operator*(const Value& r) const { return to_numeric() * r.to_numeric(); }
  Value operator*(int i) const { return mul_numeric(i); }
  Value operator*(unsigned int ui) const { return mul_numeric(ui); }
  Value operator*(long l) const { return mul_numeric(l); }
  Value operator*(unsigned long ul) const { return mul_numeric(ul); }
  Value operator*(long long ll) const { return mul_numeric(ll); }
  Value operator*(unsigned long long ull) const { return mul_numeric(ull); }
  Value operator*(double d) const { return mul_numeric(d); }
  Value operator/(const Value& r) const { return to_numeric() / r.to_numeric(); }
  Value operator/(int i) const { return div_numeric(i); }
  Value operator/(unsigned int ui) const { return div_numeric(ui); }
  Value operator/(long l) const { return div_numeric(l); }
  Value operator/(unsigned long ul) const { return div_numeric(ul); }
  Value operator/(long long ll) const { return div_numeric(ll); }
  Value operator/(unsigned long long ull) const { return div_numeric(ull); }
  Value operator/(double d) const { return div_numeric(d); }
  Value& operator+=(const Value& r) { *this = *this + r; return *this; }
  Value& operator-=(const Value& r) { *this = *this - r; return *this; }
  Value& operator*=(const Value& r) { *this = *this * r; return *this; }
  Value& operator/=(const Value& r) { *this = *this / r; return *this; }

  Value& operator=(const Value& r) noexcept { return v_ = r.v_, *this; }
  Value& operator=(Value&& r) noexcept { return v_ = std::move(r.v_), *this; }
  Value& operator=(std::nullptr_t) noexcept { return v_ = nullptr, *this; }
  Value& operator=(bool b) noexcept { return v_ = b, *this; }
  Value& operator=(int i) noexcept { return set_numeric(i); }
  Value& operator=(unsigned int ui) noexcept { return set_numeric(ui); }
  Value& operator=(long l) noexcept { return set_numeric(l); }
  Value& operator=(unsigned long ul) noexcept { return set_numeric(ul); }
  Value& operator=(long long ll) noexcept { return set_numeric(ll); }
  Value& operator=(unsigned long long ull) noexcept { return set_numeric(ull); }
  Value& operator=(double d) noexcept { return v_ = d, *this; }
  Value& operator=(const char* s) noexcept { return set_string(s); }
  Value& operator=(const std::string& s) noexcept { return v_ = s, *this; }

  bool is_truthy(void) const;
  std::string stringify(void) const;
};

}
