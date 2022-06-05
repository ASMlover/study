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

namespace loxpp::value {

class Value : public Copyable {
  enum class Type {
    NIL,
    BOOLEAN,

    INTEGER_BEG,
    INT8,
    UINT8,
    INT16,
    UINT16,
    INT32,
    UINT32,
    INT64,
    UINT64,
    INTEGER_END,

    FLOAT,
    DOUBLE,
    STRING,
  };

  using Variant = std::variant<
    nil_t,
    bool,
    i8_t,
    u8_t,
    i16_t,
    u16_t,
    i32_t,
    u32_t,
    i64_t,
    u64_t,
    float,
    double,
    str_t>;

  Type t_{Type::NIL};
  Variant v_{};
public:
  Value() noexcept : t_{Type::NIL}, v_{nullptr} {}
  Value(nil_t) noexcept : t_{Type::NIL}, v_{nullptr} {}
  Value(bool b) noexcept : t_{Type::BOOLEAN}, v_{b} {}
  Value(i8_t n) noexcept : t_{Type::INT8}, v_{n} {}
  Value(u8_t n) noexcept : t_{Type::UINT8}, v_{n} {}
  Value(i16_t n) noexcept : t_{Type::INT16}, v_{n} {}
  Value(u16_t n) noexcept : t_{Type::UINT16}, v_{n} {}
  Value(i32_t n) noexcept : t_{Type::INT32}, v_{n} {}
  Value(u32_t n) noexcept : t_{Type::UINT32}, v_{n} {}
  Value(i64_t n) noexcept : t_{Type::INT64}, v_{n} {}
  Value(u64_t n) noexcept : t_{Type::UINT64}, v_{n} {}
#if defined(LOXPP_GNUC)
  Value(long long n) noexcept : t_{Type::INT64}, v_{as_type<i64_t>(n)} {}
  Value(unsigned long long n) noexcept : t_{Type::UINT64}, v_{as_type<u64_t>(n)} {}
#endif
  Value(float n) noexcept : t_{Type::FLOAT}, v_{n} {}
  Value(double n) noexcept : t_{Type::DOUBLE}, v_{n} {}
  Value(cstr_t s) noexcept : t_{Type::STRING}, v_{str_t(s)} {}
  Value(strv_t s) noexcept : t_{Type::STRING}, v_{str_t(s)} {}
  Value(const str_t& s) noexcept : t_{Type::STRING}, v_{s} {}
  Value(const Value& r) noexcept : t_{r.t_}, v_{r.v_} {}
  Value(Value&& r) noexcept : t_{std::move(r.t_)}, v_{std::move(r.v_)} {}

  inline bool is_nil() const noexcept { return std::holds_alternative<nil_t>(v_); }
  inline bool is_boolean() const noexcept { return std::holds_alternative<bool>(v_); }
  inline bool is_int8() const noexcept { return std::holds_alternative<i8_t>(v_); }
  inline bool is_uint8() const noexcept { return std::holds_alternative<u8_t>(v_); }
  inline bool is_int16() const noexcept { return std::holds_alternative<i16_t>(v_); }
  inline bool is_uint16() const noexcept { return std::holds_alternative<u16_t>(v_); }
  inline bool is_int32() const noexcept { return std::holds_alternative<i32_t>(v_); }
  inline bool is_uint32() const noexcept { return std::holds_alternative<u32_t>(v_); }
  inline bool is_int64() const noexcept { return std::holds_alternative<i64_t>(v_); }
  inline bool is_uint64() const noexcept { return std::holds_alternative<u64_t>(v_); }
  inline bool is_integer() const noexcept { return Type::INTEGER_BEG < t_ && t_ < Type::INTEGER_END; }
  inline bool is_float() const noexcept { return std::holds_alternative<float>(v_); }
  inline bool is_double() const noexcept { return std::holds_alternative<double>(v_); }
  inline bool is_decimal() const noexcept { return is_float() || is_double(); }
  inline bool is_numeric() const noexcept { return is_integer() || is_decimal(); }
  inline bool is_string() const noexcept { return std::holds_alternative<str_t>(v_); }

  inline bool as_boolean() const noexcept { return std::get<bool>(v_); }
  inline i8_t as_int8() const noexcept { return std::get<i8_t>(v_); }
  inline u8_t as_uint8() const noexcept { return std::get<u8_t>(v_); }
  inline i16_t as_int16() const noexcept { return std::get<i16_t>(v_); }
  inline u16_t as_uint16() const noexcept { return std::get<u16_t>(v_); }
  inline i32_t as_int32() const noexcept { return std::get<i32_t>(v_); }
  inline u32_t as_uint32() const noexcept { return std::get<u32_t>(v_); }
  inline i64_t as_int64() const noexcept { return std::get<i64_t>(v_); }
  inline u64_t as_uint64() const noexcept { return std::get<u64_t>(v_); }
  i64_t as_integer() const noexcept;
  inline float as_float() const noexcept { return std::get<float>(v_); }
  inline double as_double() const noexcept { return std::get<double>(v_); }
  double as_decimal() const noexcept;
  inline str_t as_string() const noexcept { return std::get<str_t>(v_); }
};

}
