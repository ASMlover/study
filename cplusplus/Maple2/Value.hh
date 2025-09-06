// Copyright (c) 2025 ASMlover. All rights reserved.
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

#include "Common.hh"
#include "ValueHelper.hh"

namespace ms {

class Value final : public Copyable {
  ValueType v_{};

  template <typename T> inline double number_cast(T x) noexcept { return as_type<double>(x); }
  template <typename T> inline str_t string_cast(T x) noexcept { return str_t{x}; }
public:
  Value() noexcept : v_{nullptr} {}
  Value(nil_t) noexcept : v_{nullptr} {}
  Value(bool b) noexcept : v_{b} {}
  Value(i8_t n) noexcept : v_{number_cast(n)} {}
  Value(u8_t n) noexcept : v_{number_cast(n)} {}
  Value(i16_t n) noexcept : v_{number_cast(n)} {}
  Value(u16_t n) noexcept : v_{number_cast(n)} {}
  Value(i32_t n) noexcept : v_{number_cast(n)} {}
  Value(u32_t n) noexcept : v_{number_cast(n)} {}
  Value(i64_t n) noexcept : v_{number_cast(n)} {}
  Value(u64_t n) noexcept : v_{number_cast(n)} {}
#if defined (MAPLE_GUNC)
  Value(long long ll) noexcept : v_{number_cast(ll)} {}
  Value(unsigned long long ull) noexcept : v_{number_cast(ull)} {}
#endif
  Value(float f) noexcept : v_{number_cast(f)} {}
  Value(double d) noexcept : v_{d} {}
  Value(cstr_t s) noexcept : v_{string_cast(s)} {}
  Value(strv_t s) noexcept : v_{string_cast(s)} {}
  Value(const str_t& s) noexcept : v_{s} {}
  Value(const CallablePtr& c) noexcept : v_{c} {}
  Value(const InstancePtr& i) noexcept : v_{i} {}
  Value(const ModulePtr& m) noexcept : v_{m} {}
  Value(const Value& r) noexcept : v_{r.v_} {}
  Value(Value&& r) noexcept : v_{std::move(r.v_)} {}

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

  inline bool is_nil() const noexcept { return std::holds_alternative<nil_t>(v_); }
  inline bool is_boolean() const noexcept { return std::holds_alternative<bool>(v_); }
  inline bool is_number() const noexcept { return std::holds_alternative<double>(v_); }
  inline bool is_string() const noexcept { return std::holds_alternative<str_t>(v_); }
  inline bool is_callable() const noexcept { return std::holds_alternative<CallablePtr>(v_); }
  inline bool is_instance() const noexcept { return std::holds_alternative<InstancePtr>(v_); }
  inline bool is_module() const noexcept { return std::holds_alternative<ModulePtr>(v_); }

  inline bool as_boolean() const noexcept { return std::get<bool>(v_); }
  inline double as_number() const noexcept { return std::get<double>(v_); }
  inline const str_t& as_string() const noexcept { return std::get<str_t>(v_); }
  inline CallablePtr as_callable() const noexcept { return std::get<CallablePtr>(v_); }
  inline InstancePtr as_instance() const noexcept { return std::get<InstancePtr>(v_); }
  inline ModulePtr as_module() const noexcept { return std::get<ModulePtr>(v_); }

  inline operator bool() const noexcept { return as_boolean(); }
  inline operator double() const noexcept { return as_number(); }
  inline operator str_t() const noexcept { return as_string(); }
  inline operator CallablePtr() const noexcept { return as_callable(); }
  inline operator InstancePtr() const noexcept { return as_instance(); }
  inline operator ModulePtr() const noexcept { return as_module(); }

  inline bool operator==(const Value& r) const noexcept {
    return (is_number() && r.is_number()) ? as_number() == r.as_number() : v_ == r.v_;
  }

  inline bool operator!=(const Value& r) const noexcept { return !(*this == r); }
  inline bool operator<(const Value& r) const noexcept { return as_number() < r.as_number(); }
  inline bool operator<=(const Value& r) const noexcept { return as_number() <= r.as_number(); }
  inline bool operator>(const Value& r) const noexcept { return as_number() > r.as_number(); }
  inline bool operator>=(const Value& r) const noexcept { return as_number() >= r.as_number(); }

  inline Value operator+(const Value& r) const noexcept {
    if (is_string() && r.is_string())
      return as_string() + r.as_string();
    return as_number() + r.as_number();
  }

  inline Value operator-(const Value& r) const noexcept { return as_number() - r.as_number(); }
  inline Value operator*(const Value& r) const noexcept { return as_number() * r.as_number(); }
  inline Value operator/(const Value& r) const noexcept { return as_number() / r.as_number(); }
  inline Value operator-() const noexcept { return -as_number(); }
  inline Value operator!() const noexcept { return !is_truthy(); }

  inline bool is_abs_equal(const Value& r) const noexcept { return v_ == r.v_; }
  inline bool is_equal(const Value& r) const noexcept { return (this == &r) || (*this == r); }

  bool is_truthy() const noexcept;
  str_t stringify() const noexcept;
};

inline std::ostream& operator<<(std::ostream& out, const Value& v) noexcept {
  return out << v.stringify();
}

}
