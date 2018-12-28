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
  Value& operator=(int i) noexcept { return operator=(static_cast<double>(i)); }
  Value& operator=(unsigned int ui) noexcept { return operator=(static_cast<double>(ui)); }
  Value& operator=(long l) noexcept { return operator=(static_cast<double>(l)); }
  Value& operator=(unsigned long ul) noexcept { return operator=(static_cast<double>(ul)); }
  Value& operator=(long long ll) noexcept { return operator=(static_cast<double>(ll)); }
  Value& operator=(unsigned long long ull) noexcept { return operator=(static_cast<double>(ull)); }
  Value& operator=(double d) noexcept { return v_ = d, *this; }
  Value& operator=(const char* s) noexcept { return operator=(static_cast<const std::string&>(s)); }
  Value& operator=(const std::string& s) noexcept { return v_ = s, *this; }

  bool is_truthy(void) const;
  std::string stringify(void) const;
};

}
