// Copyright (c) 2026 ASMlover. All rights reserved.
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

#include <vector>
#include "Types.hh"

namespace ms {

class Object;  // forward declare

#ifdef MAPLE_NAN_BOXING

// NaN-boxing: encode nil, bool, and Object* inside IEEE 754 quiet NaN payload.
// Layout (64 bits):
//   Number  : raw IEEE 754 double (any bit pattern where (bits & QNAN) != QNAN)
//   nil     : QNAN | TAG_NIL
//   true    : QNAN | TAG_TRUE
//   false   : QNAN | TAG_FALSE
//   Object* : SIGN_BIT | QNAN | 48-bit pointer
//
// This shrinks Value from 16 bytes (std::variant) to 8 bytes and replaces
// branching type checks with bitwise operations.

inline constexpr u64_t kSIGN_BIT = static_cast<u64_t>(0x8000000000000000);
inline constexpr u64_t kQNAN     = static_cast<u64_t>(0x7FFC000000000000);
inline constexpr u64_t kTAG_NIL   = 1;
inline constexpr u64_t kTAG_FALSE = 2;
inline constexpr u64_t kTAG_TRUE  = 3;
// Integer tag: QNAN + bit 49 set, payload in bits 0-47 (48-bit signed)
inline constexpr u64_t kINT_TAG   = kQNAN | (static_cast<u64_t>(1) << 49);
inline constexpr u64_t kINT_MASK  = 0x0000FFFFFFFFFFFF; // lower 48 bits
inline constexpr u64_t kINT_SIGN  = static_cast<u64_t>(1) << 47; // sign bit within payload

class Value {
  u64_t bits_;

  explicit Value(u64_t bits) noexcept : bits_(bits) {}
public:
  // Constructors
  Value() noexcept : bits_(kQNAN | kTAG_NIL) {}
  Value(bool b) noexcept : bits_(kQNAN | (b ? kTAG_TRUE : kTAG_FALSE)) {}
  Value(double d) noexcept;
  Value(i64_t i) noexcept;
  Value(Object* obj) noexcept;

  // Type checks
  bool is_nil() const noexcept { return bits_ == (kQNAN | kTAG_NIL); }
  bool is_boolean() const noexcept { return (bits_ | 1) == (kQNAN | kTAG_TRUE); }
  bool is_double() const noexcept { return (bits_ & kQNAN) != kQNAN; }
  bool is_integer() const noexcept {
    return (bits_ & (kSIGN_BIT | kQNAN | (static_cast<u64_t>(1) << 49)))
        == kINT_TAG;
  }
  bool is_number() const noexcept { return is_double() || is_integer(); }
  bool is_object() const noexcept { return (bits_ & (kQNAN | kSIGN_BIT)) == (kQNAN | kSIGN_BIT); }

  // Value extraction
  bool as_boolean() const noexcept { return bits_ == (kQNAN | kTAG_TRUE); }
  double as_number() const noexcept;
  i64_t as_integer() const noexcept;
  Object* as_object() const noexcept;

  // Semantic operations
  bool is_truthy() const noexcept;
  bool is_equal(const Value& other) const noexcept;
  str_t stringify() const noexcept;

  // Convenience object type checks
  bool is_string() const noexcept;
  bool is_function() const noexcept;
  bool is_closure() const noexcept;
  bool is_class() const noexcept;
  bool is_instance() const noexcept;
  bool is_list() const noexcept;
  bool is_map() const noexcept;
  bool is_tuple() const noexcept;
};

#else // !MAPLE_NAN_BOXING

#include <variant>

using ValueStorage = std::variant<std::monostate, bool, double, i64_t, Object*>;

class Value {
  ValueStorage storage_;
public:
  // Constructors
  Value() noexcept : storage_(std::monostate{}) {}
  Value(bool b) noexcept : storage_(b) {}
  Value(double d) noexcept : storage_(d) {}
  Value(i64_t i) noexcept : storage_(i) {}
  Value(Object* obj) noexcept : storage_(obj) {}

  // Type checks
  bool is_nil() const noexcept;
  bool is_boolean() const noexcept;
  bool is_double() const noexcept;
  bool is_integer() const noexcept;
  bool is_number() const noexcept;
  bool is_object() const noexcept;

  // Value extraction
  bool as_boolean() const noexcept;
  double as_number() const noexcept;
  i64_t as_integer() const noexcept;
  Object* as_object() const noexcept;

  // Semantic operations
  bool is_truthy() const noexcept;
  bool is_equal(const Value& other) const noexcept;
  str_t stringify() const noexcept;

  // Convenience object type checks
  bool is_string() const noexcept;
  bool is_function() const noexcept;
  bool is_closure() const noexcept;
  bool is_class() const noexcept;
  bool is_instance() const noexcept;
  bool is_list() const noexcept;
  bool is_map() const noexcept;
  bool is_tuple() const noexcept;
};

#endif // MAPLE_NAN_BOXING

// operator== for convenience
inline bool operator==(const Value& a, const Value& b) noexcept {
  return a.is_equal(b);
}

inline bool operator!=(const Value& a, const Value& b) noexcept {
  return !a.is_equal(b);
}

// ostream support
std::ostream& operator<<(std::ostream& os, const Value& value);

} // namespace ms
