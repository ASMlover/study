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

#include <memory>
#include <string>
#include <variant>

struct Callable;
class LoxInstance;

class Value {
  using CallablePtr = std::shared_ptr<Callable>;
  using InstancePtr = std::shared_ptr<LoxInstance>;
  using VariantType = std::variant<
    std::nullptr_t, bool, double, std::string, CallablePtr, InstancePtr>;

  VariantType v_{};

  template <typename T> Value& set_numeric(T&& x) noexcept {
    return v_ = static_cast<double>(x), *this;
  }

  Value& set_string(const std::string& s) noexcept {
    return v_ = s, *this;
  }

  Value add_operation(const Value& r) const noexcept {
    if (is_numeric() && r.is_numeric())
      return (double)*this + (double)r;
    else if (is_string() && r.is_string())
      return (std::string)*this + (std::string)r;
    return nullptr;
  }

  template <typename T> Value add_numeric(T&& x) const noexcept {
    return to_numeric() + static_cast<double>(x);
  }
  template <typename T> Value sub_numeric(T&& x) const noexcept {
    return to_numeric() - static_cast<double>(x);
  }
  template <typename T> Value mul_numeric(T&& x) const noexcept {
    return to_numeric() * static_cast<double>(x);
  }
  template <typename T> Value div_numeric(T&& x) const noexcept {
    return to_numeric() / static_cast<double>(x);
  }
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
  Value(const CallablePtr& callable) noexcept : v_(callable) {}
  Value(const InstancePtr& instance) noexcept : v_(instance) {}
  Value(const Value& r) noexcept : v_(r.v_) {}
  Value(Value&& r) noexcept : v_(std::move(r.v_)) {}

  bool is_nil(void) const { return std::holds_alternative<std::nullptr_t>(v_); }
  bool is_boolean(void) const { return std::holds_alternative<bool>(v_); }
  bool is_numeric(void) const { return std::holds_alternative<double>(v_); }
  bool is_string(void) const { return std::holds_alternative<std::string>(v_); }
  bool is_callable(void) const { return std::holds_alternative<CallablePtr>(v_); }
  bool is_instance(void) const { return std::holds_alternative<InstancePtr>(v_); }

  bool to_boolean(void) const { return std::get<bool>(v_); }
  double to_numeric(void) const { return std::get<double>(v_); }
  std::string to_string(void) const { return std::get<std::string>(v_); }
  CallablePtr to_callable(void) const { return std::get<CallablePtr>(v_); }
  InstancePtr to_instance(void) const { return std::get<InstancePtr>(v_); }

  operator bool(void) const { return std::get<bool>(v_); }
  operator double(void) const { return std::get<double>(v_); }
  operator std::string(void) const { return std::get<std::string>(v_); }
  operator CallablePtr(void)  const { return std::get<CallablePtr>(v_); }
  operator InstancePtr(void)  const { return std::get<InstancePtr>(v_); }

  Value operator+(const Value& r) const { return add_operation(r); }
  Value operator-(const Value& r) const { return sub_numeric(r); }
  Value operator*(const Value& r) const { return mul_numeric(r); }
  Value operator/(const Value& r) const { return div_numeric(r); }
  Value& operator+=(const Value& r) { return *this = *this + r, *this; }
  Value& operator-=(const Value& r) { return *this = *this - r, *this; }
  Value& operator*=(const Value& r) { return *this = *this * r, *this; }
  Value& operator/=(const Value& r) { return *this = *this / r, *this; }
  Value operator-(void) const { return -to_numeric(); }
  bool operator!(void) const { return !is_truthy(); }

  bool operator==(const Value& r) const { return v_ == r.v_; }
  bool operator!=(const Value& r) const { return v_ != r.v_; }
  bool operator<(const Value& r) const { return v_ < r.v_; }
  bool operator<=(const Value& r) const { return v_ <= r.v_; }
  bool operator>(const Value& r) const { return v_ > r.v_; }
  bool operator>=(const Value& r) const { return v_ >= r.v_; }

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

  std::string stringify(void) const;
  bool is_truthy(void) const;
};
