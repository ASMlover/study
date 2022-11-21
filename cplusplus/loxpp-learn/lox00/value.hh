// Copyright (c) 2022 ASMlover. All rights reserved.
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

#include <variant>
#include "common.hh"

namespace loxpp::callable {

interface Callable;
class Instance;

}

namespace loxpp::value {

using CallablePtr = std::shared_ptr<callable::Callable>;
using InstancePtr = std::shared_ptr<callable::Instance>;

class Value : public Copyable {
  std::variant<nil_t, bool, double, str_t, CallablePtr, InstancePtr> v_{};

  template <typename T> inline double numeric_cast(T x) noexcept { return as_type<double>(x); }
  template <typename T> inline str_t string_cast(T x) noexcept { return str_t(x); }
public:
  Value() noexcept : v_{nullptr} {}
  Value(nil_t) noexcept : v_{nullptr} {}
  Value(bool b) noexcept : v_{b} {}
  Value(i8_t n) noexcept : v_{numeric_cast(n)} {}
  Value(u8_t n) noexcept : v_{numeric_cast(n)} {}
  Value(i16_t n) noexcept : v_{numeric_cast(n)} {}
  Value(u16_t n) noexcept : v_{numeric_cast(n)} {}
  Value(i32_t n) noexcept : v_{numeric_cast(n)} {}
  Value(u32_t n) noexcept : v_{numeric_cast(n)} {}
  Value(i64_t n) noexcept : v_{numeric_cast(n)} {}
  Value(u64_t n) noexcept : v_{numeric_cast(n)} {}
#if defined(LOXPP_GNUC)
  Value(long long n) noexcept : v_{numeric_cast(n)} {}
  Value(unsigned long long n) noexcept : v_{numeric_cast(n)} {}
#endif
  Value(float n) noexcept : v_{numeric_cast(n)} {}
  Value(double n) noexcept : v_{n} {}
  Value(cstr_t s) noexcept : v_{string_cast(s)} {}
  Value(strv_t s) noexcept : v_{string_cast(s)} {}
  Value(const str_t& s) noexcept : v_{s} {}
  Value(const CallablePtr& c) noexcept : v_{c} {}
  Value(const InstancePtr& i) noexcept : v_{i} {}
  Value(const Value& r) noexcept : v_{r.v_} {}
  Value(Value&& r) noexcept : v_{std::move(r.v_)} {}

  inline bool is_nil() const noexcept { return std::holds_alternative<nil_t>(v_); }
  inline bool is_boolean() const noexcept { return std::holds_alternative<bool>(v_); }
  inline bool is_numeric() const noexcept { return std::holds_alternative<double>(v_); }
  inline bool is_string() const noexcept { return std::holds_alternative<str_t>(v_); }
  inline bool is_callable() const noexcept { return std::holds_alternative<CallablePtr>(v_); }
  inline bool is_instance() const noexcept { return std::holds_alternative<InstancePtr>(v_); }

  inline bool as_boolean() const noexcept { return std::get<bool>(v_); }
  inline double as_numeric() const noexcept { return std::get<double>(v_); }
  inline str_t as_string() const noexcept { return std::get<str_t>(v_); }
  inline CallablePtr as_callable() const noexcept { return std::get<CallablePtr>(v_); }
  inline InstancePtr as_instance() const noexcept { return std::get<InstancePtr>(v_); }

  inline bool operator==(const Value& r) const noexcept {
    return (is_numeric() && r.is_numeric()) ? as_numeric() == r.as_numeric() : v_ == r.v_;
  }

  inline bool operator!=(const Value& r) const noexcept { return !(*this == r); }
  inline bool operator<(const Value& r) const noexcept { return as_numeric() < r.as_numeric(); }
  inline bool operator<=(const Value& r) const noexcept { return as_numeric() <= r.as_numeric(); }
  inline bool operator>(const Value& r) const noexcept { return as_numeric() > r.as_numeric(); }
  inline bool operator>=(const Value& r) const noexcept { return as_numeric() >= r.as_numeric(); }

  inline Value& operator=(const Value& r) noexcept { if (this != &r) v_ = r.v_; return *this; }
  inline Value& operator=(Value&& r) noexcept { if (this != &r) v_ = std::move(r.v_); return *this; }

  inline Value operator+(const Value& r) const noexcept {
    if (is_string() && r.is_string())
      return as_string() + r.as_string();
    return as_numeric() + r.as_numeric();
  }

  inline Value operator-(const Value& r) const noexcept { return as_numeric() - r.as_numeric(); }
  inline Value operator*(const Value& r) const noexcept { return as_numeric() * r.as_numeric(); }
  inline Value operator/(const Value& r) const noexcept { return as_numeric() / r.as_numeric(); }
  inline Value operator-() const noexcept { return -as_numeric(); }
  inline Value operator!() const noexcept { return !is_truthy(); }

  inline bool is_abs_equal(const Value& r) const noexcept { return v_ == r.v_; }
  inline bool is_equal(const Value& r) const noexcept { return (this == &r) || (*this == r); }

  bool is_truthy() const noexcept;
  str_t stringify() const noexcept;
};

inline std::ostream& operator<<(std::ostream& out, const Value& val) noexcept {
  return out << val.stringify();
}

}
