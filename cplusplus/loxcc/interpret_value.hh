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

#include <memory>
#include <ostream>
#include <variant>
#include "common.hh"

namespace loxcc::interpret {

struct Callable;
class Instance;

using CallablePtr = std::shared_ptr<Callable>;
using InstancePtr = std::shared_ptr<Instance>;

class Value final : private UnCopyable {
  std::variant<
    nil_t,
    bool,
    double,
    str_t,
    CallablePtr,
    InstancePtr> v_{};
public:
  Value(void) noexcept : v_(nullptr) {}
  Value(nil_t) noexcept : v_(nullptr) {}
  Value(bool b) noexcept : v_(b) {}
  Value(i8_t x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(u8_t x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(i16_t x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(u16_t x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(i32_t x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(u32_t x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(i64_t x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(u64_t x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(float x) noexcept : v_(Xt::to_decimal(x)) {}
  Value(double d) noexcept : v_(d) {}
  Value(const char* s) noexcept : v_(str_t(s)) {}
  Value(const str_t& s) noexcept : v_(s) {}
  Value(const CallablePtr& c) noexcept : v_(c) {}
  Value(const InstancePtr& i) noexcept : v_(i) {}
  Value(const Value& r) noexcept : v_(r.v_) {}
  Value(Value&& r) noexcept : v_(std::move(r.v_)) {}

  inline Value& operator=(const Value& r) noexcept {
    if (this != &r)
      v_ = r.v_;
    return *this;
  }

  inline Value& operator=(Value&& r) noexcept {
    if (this != &r)
      v_ = std::move(r.v_);
    return *this;
  }

  inline bool is_nil(void) const { return std::holds_alternative<nil_t>(v_); }
  inline bool is_boolean(void) const { return std::holds_alternative<bool>(v_); }
  inline bool is_numeric(void) const { return std::holds_alternative<double>(v_); }
  inline bool is_string(void) const { return std::holds_alternative<str_t>(v_); }
  inline bool is_callable(void) const { return std::holds_alternative<CallablePtr>(v_); }
  inline bool is_instance(void) const { return std::holds_alternative<InstancePtr>(v_); }

  inline bool as_boolean(void) const { return std::get<bool>(v_); }
  inline double as_numeric(void) const { return std::get<double>(v_); }
  inline str_t as_string(void) const { return std::get<str_t>(v_); }
  inline CallablePtr as_callable(void) const { return std::get<CallablePtr>(v_); }
  inline InstancePtr as_instance(void) const { return std::get<InstancePtr>(v_); }

  inline bool operator==(const Value& r) const noexcept {
    if (is_numeric() && r.is_numeric())
      return as_numeric() == r.as_numeric();
    return v_ == r.v_;
  }

  inline bool operator!=(const Value& r) const noexcept { return !(*this == r); }
  inline bool operator>(const Value& r) const noexcept { return as_numeric() > r.as_numeric(); }
  inline bool operator>=(const Value& r) const noexcept { return as_numeric() >= r.as_numeric(); }
  inline bool operator<(const Value& r) const noexcept { return as_numeric() < r.as_numeric(); }
  inline bool operator<=(const Value& r) const noexcept { return as_numeric() <= r.as_numeric(); }

  inline Value operator+(const Value& r) const noexcept {
    if (is_string() && r.is_string())
      return as_string() + r.as_string();
    return as_numeric() + r.as_numeric();
  }

  inline Value operator-(const Value& r) const noexcept { return as_numeric() - r.as_numeric(); }
  inline Value operator*(const Value& r) const noexcept { return as_numeric() * r.as_numeric(); }
  inline Value operator/(const Value& r) const noexcept { return as_numeric() / r.as_numeric(); }
  inline Value operator-(void) const noexcept { return -as_numeric(); }
  inline Value operator!(void) const noexcept { return !is_truthy(); }

  inline bool is_abs_equal(const Value& r) const { return v_ == r.v_; }
  inline bool is_equal(const Value& r) const { return (this == &r) || (*this == r); }

  bool is_truthy(void) const;
  str_t stringify(void) const;
};

inline std::ostream& operator<<(std::ostream& out, const Value& val) {
  return out << val.stringify();
}

}
