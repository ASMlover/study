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
#include <bit>
#include <cmath>
#include <iostream>
#include <format>
#include "Value.hh"
#include "Object.hh"

namespace ms {

#ifdef MAPLE_NAN_BOXING

// --- NaN-boxing constructors & accessors ---

Value::Value(double d) noexcept
    : bits_(std::bit_cast<u64_t>(d)) {}

Value::Value(i64_t i) noexcept
    : bits_(kINT_TAG | (static_cast<u64_t>(i) & kINT_MASK)) {}

Value::Value(Object* obj) noexcept
    : bits_(kSIGN_BIT | kQNAN | static_cast<u64_t>(reinterpret_cast<uintptr_t>(obj))) {}

double Value::as_number() const noexcept {
  if (is_integer()) return static_cast<double>(as_integer());
  return std::bit_cast<double>(bits_);
}

i64_t Value::as_integer() const noexcept {
  i64_t raw = static_cast<i64_t>(bits_ & kINT_MASK);
  // Sign-extend from bit 47
  if (raw & static_cast<i64_t>(kINT_SIGN)) {
    raw |= static_cast<i64_t>(~kINT_MASK);
  }
  return raw;
}

Object* Value::as_object() const noexcept {
  return reinterpret_cast<Object*>(static_cast<uintptr_t>(bits_ & ~(kSIGN_BIT | kQNAN)));
}

bool Value::is_truthy() const noexcept {
  if (is_nil()) return false;
  if (is_boolean()) return as_boolean();
  if (is_integer()) return as_integer() != 0;
  if (is_double()) return as_number() != 0.0;
  return true;
}

bool Value::is_equal(const Value& other) const noexcept {
  // Both integers: compare directly
  if (is_integer() && other.is_integer()) {
    return as_integer() == other.as_integer();
  }
  // Cross-type int/double: promote to double
  if (is_number() && other.is_number()) {
    return as_number() == other.as_number();
  }
  // Structural equality for tuples
  if (is_tuple() && other.is_tuple()) {
    auto* a = as_obj<ObjTuple>(as_object());
    auto* b = as_obj<ObjTuple>(other.as_object());
    if (a->len() != b->len()) return false;
    for (sz_t i = 0; i < a->len(); i++) {
      if (!a->elements()[i].is_equal(b->elements()[i])) return false;
    }
    return true;
  }
  return bits_ == other.bits_;
}

str_t Value::stringify() const noexcept {
  if (is_nil()) return "nil";
  if (is_boolean()) return as_boolean() ? "true" : "false";
  if (is_integer()) {
    return std::format("{}", as_integer());
  }
  if (is_double()) {
    double val = as_number();
    if (val == std::floor(val) && !std::isinf(val) && !std::isnan(val)) {
      return std::format("{}", static_cast<long long>(val));
    }
    return std::format("{:g}", val);
  }
  if (is_object()) {
    return object_stringify(as_object());
  }
  return "nil";
}

// Convenience object type checks
bool Value::is_string() const noexcept { return is_obj_type(*this, ObjectType::OBJ_STRING); }
bool Value::is_function() const noexcept { return is_obj_type(*this, ObjectType::OBJ_FUNCTION); }
bool Value::is_closure() const noexcept { return is_obj_type(*this, ObjectType::OBJ_CLOSURE); }
bool Value::is_class() const noexcept { return is_obj_type(*this, ObjectType::OBJ_CLASS); }
bool Value::is_instance() const noexcept { return is_obj_type(*this, ObjectType::OBJ_INSTANCE); }
bool Value::is_list() const noexcept { return is_obj_type(*this, ObjectType::OBJ_LIST); }
bool Value::is_map() const noexcept { return is_obj_type(*this, ObjectType::OBJ_MAP); }
bool Value::is_tuple() const noexcept { return is_obj_type(*this, ObjectType::OBJ_TUPLE); }

#else // !MAPLE_NAN_BOXING

bool Value::is_nil() const noexcept {
  return std::holds_alternative<std::monostate>(storage_);
}

bool Value::is_boolean() const noexcept {
  return std::holds_alternative<bool>(storage_);
}

bool Value::is_double() const noexcept {
  return std::holds_alternative<double>(storage_);
}

bool Value::is_integer() const noexcept {
  return std::holds_alternative<i64_t>(storage_);
}

bool Value::is_number() const noexcept {
  return is_double() || is_integer();
}

bool Value::is_object() const noexcept {
  return std::holds_alternative<Object*>(storage_);
}

bool Value::as_boolean() const noexcept {
  return std::get<bool>(storage_);
}

double Value::as_number() const noexcept {
  if (is_integer()) return static_cast<double>(std::get<i64_t>(storage_));
  return std::get<double>(storage_);
}

i64_t Value::as_integer() const noexcept {
  return std::get<i64_t>(storage_);
}

Object* Value::as_object() const noexcept {
  return std::get<Object*>(storage_);
}

bool Value::is_truthy() const noexcept {
  if (is_nil()) return false;
  if (is_boolean()) return as_boolean();
  if (is_integer()) return as_integer() != 0;
  if (is_double()) return as_number() != 0.0;
  return true;
}

bool Value::is_equal(const Value& other) const noexcept {
  if (is_nil() && other.is_nil()) return true;
  if (is_nil() || other.is_nil()) return false;

  // Both integers
  if (is_integer() && other.is_integer()) return as_integer() == other.as_integer();
  // Cross-type int/double
  if (is_number() && other.is_number()) return as_number() == other.as_number();

  if (is_boolean() && other.is_boolean()) return as_boolean() == other.as_boolean();
  if (is_object() && other.is_object()) {
    // Structural equality for tuples
    if (is_tuple() && other.is_tuple()) {
      auto* a = as_obj<ObjTuple>(as_object());
      auto* b = as_obj<ObjTuple>(other.as_object());
      if (a->len() != b->len()) return false;
      for (sz_t i = 0; i < a->len(); i++) {
        if (!a->elements()[i].is_equal(b->elements()[i])) return false;
      }
      return true;
    }
    return as_object() == other.as_object();
  }

  return false;
}

str_t Value::stringify() const noexcept {
  if (is_nil()) return "nil";
  if (is_boolean()) return as_boolean() ? "true" : "false";
  if (is_integer()) {
    return std::format("{}", as_integer());
  }
  if (is_double()) {
    double val = as_number();
    if (val == std::floor(val) && !std::isinf(val) && !std::isnan(val)) {
      return std::format("{}", static_cast<long long>(val));
    }
    return std::format("{:g}", val);
  }
  if (is_object()) {
    return object_stringify(as_object());
  }
  return "nil";
}

// Convenience object type checks
bool Value::is_string() const noexcept { return is_obj_type(*this, ObjectType::OBJ_STRING); }
bool Value::is_function() const noexcept { return is_obj_type(*this, ObjectType::OBJ_FUNCTION); }
bool Value::is_closure() const noexcept { return is_obj_type(*this, ObjectType::OBJ_CLOSURE); }
bool Value::is_class() const noexcept { return is_obj_type(*this, ObjectType::OBJ_CLASS); }
bool Value::is_instance() const noexcept { return is_obj_type(*this, ObjectType::OBJ_INSTANCE); }
bool Value::is_list() const noexcept { return is_obj_type(*this, ObjectType::OBJ_LIST); }
bool Value::is_map() const noexcept { return is_obj_type(*this, ObjectType::OBJ_MAP); }
bool Value::is_tuple() const noexcept { return is_obj_type(*this, ObjectType::OBJ_TUPLE); }

#endif // MAPLE_NAN_BOXING

std::ostream& operator<<(std::ostream& os, const Value& value) {
  os << value.stringify();
  return os;
}

} // namespace ms
