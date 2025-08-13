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

#include <stdexcept>
#include <format>
#include "Common.hh"
#include "ValueHelper.hh"

namespace ms {

class Value final : public Copyable {
  ValueSpecial v_{std::monostate{}};

  template <typename T> inline double number_cast(T x) noexcept { return as_type<double>(x); }
  template <typename T> inline str_t string_cast(T x) noexcept { return str_t{x}; }

  inline void check_type(ValueType expected, const str_t& type_name) const {
    if (value_type() != expected)
      throw std::runtime_error(std::format("Expected {} but got {}", type_name, name_of_type()));
  }

  str_t name_of_type() const noexcept;
public:
  Value() noexcept : v_{std::monostate{}} {}
  Value(nil_t) noexcept : v_{std::monostate{}} {}
  Value(bool b) noexcept : v_{b} {}
  Value(i8_t i) noexcept : v_{number_cast(i)} {}
  Value(u8_t i) noexcept : v_{number_cast(i)} {}
  Value(i16_t i) noexcept : v_{number_cast(i)} {}
  Value(u16_t i) noexcept : v_{number_cast(i)} {}
  Value(i32_t i) noexcept : v_{number_cast(i)} {}
  Value(u32_t i) noexcept : v_{number_cast(i)} {}
  Value(i64_t i) noexcept : v_{number_cast(i)} {}
  Value(u64_t i) noexcept : v_{number_cast(i)} {}
#if defined(MAPLE_GNUC)
  Value(long long ll) noexcept : v_{number_cast(ll)} {}
  Value(unsigned long long ull) noexcept : v_{number_cast(ull)} {}
#endif
  Value(float f) noexcept : v_{number_cast(f)} {}
  Value(double d) noexcept : v_{d} {}
  Value(const str_t& s) noexcept : v_{s} {}
  Value(strv_t s) noexcept : v_{string_cast(s)} {}
  Value(cstr_t s) noexcept : v_{string_cast(s)} {}
  Value(FunctionPtr f) noexcept : v_{std::move(f)} {}
  Value(ClassPtr c) noexcept : v_{std::move(c)} {}
  Value(InstancePtr i) noexcept : v_{std::move(i)} {}
  Value(ModulePtr m) noexcept : v_{std::move(m)} {}
  Value(const Value& other) noexcept : v_{other.v_} {}
  Value(Value&& other) noexcept : v_{std::move(other.v_)} {}

  Value& operator=(const Value& other) noexcept {
    if (this != &other)
      v_ = other.v_;
    return *this;
  }

  Value& operator=(Value&& other) noexcept {
    if (this != &other)
      v_ = std::move(other.v_);
    return *this;
  }

  inline ValueType value_type() const noexcept { return as_type<ValueType>(v_.index()); }

  inline bool is_nil() const noexcept { return value_type() == ValueType::NIL; }
  inline bool is_boolean() const noexcept { return value_type() == ValueType::BOOLEAN; }
  inline bool is_number() const noexcept { return value_type() == ValueType::NUMBER; }
  inline bool is_string() const noexcept { return value_type() == ValueType::STRING; }
  inline bool is_function() const noexcept { return value_type() == ValueType::FUNCTION; }
  inline bool is_class() const noexcept { return value_type() == ValueType::CLASS; }
  inline bool is_instance() const noexcept { return value_type() == ValueType::INSTANCE; }
  inline bool is_module() const noexcept { return value_type() == ValueType::MODULE; }

  inline bool as_boolean() const noexcept { return std::get<bool>(v_); }
  inline double as_number() const noexcept { return std::get<double>(v_); }
  inline const str_t& as_string() const noexcept { return std::get<str_t>(v_); }
  inline FunctionPtr as_function() const noexcept { return std::get<FunctionPtr>(v_); }
  inline ClassPtr as_class() const noexcept { return std::get<ClassPtr>(v_); }
  inline InstancePtr as_instance() const noexcept { return std::get<InstancePtr>(v_); }
  inline ModulePtr as_module() const noexcept { return std::get<ModulePtr>(v_); }

  inline bool operator==(const Value& other) const noexcept {
    return (is_number() && other.is_number()) ? as_number() == other.as_number() : v_ == other.v_;
  }

  inline bool operator!=(const Value& other) const noexcept { return !(*this == other); }
  inline bool operator<(const Value& other) const noexcept { return as_number() < other.as_number(); }
  inline bool operator<=(const Value& other) const noexcept { return as_number() <= other.as_number(); }
  inline bool operator>(const Value& other) const noexcept { return as_number() > other.as_number(); }
  inline bool operator>=(const Value& other) const noexcept { return as_number() >= other.as_number(); }

  inline Value operator+(const Value& other) const noexcept {
    if (is_string() && other.is_string())
      return as_string() + other.as_string();
    return as_number() + other.as_number();
  }

  inline Value operator-(const Value& other) const noexcept { return as_number() - other.as_number(); }
  inline Value operator*(const Value& other) const noexcept { return as_number() * other.as_number(); }
  inline Value operator/(const Value& other) const noexcept { return as_number() / other.as_number(); }
  inline Value operator-() const noexcept { return -as_number(); }
  inline Value operator!() const noexcept { return !is_truthy(); }

  inline bool is_abs_equal(const Value& other) const noexcept { return v_ == other.v_; }
  inline bool is_equal(const Value& other) const noexcept { return (this == &other) || (*this == other); }

  bool is_truthy() const noexcept;
  str_t stringify() const noexcept;
};

inline std::ostream& operator<<(std::ostream& out, const Value& v) noexcept {
  return out << v.stringify();
}

}
