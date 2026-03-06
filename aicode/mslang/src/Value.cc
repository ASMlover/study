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
#include <cmath>
#include <iostream>
#include <format>
#include "Value.hh"
#include "Object.hh"

namespace ms {

bool Value::is_nil() const noexcept {
  return std::holds_alternative<std::monostate>(storage_);
}

bool Value::is_boolean() const noexcept {
  return std::holds_alternative<bool>(storage_);
}

bool Value::is_number() const noexcept {
  return std::holds_alternative<double>(storage_);
}

bool Value::is_object() const noexcept {
  return std::holds_alternative<Object*>(storage_);
}

bool Value::as_boolean() const noexcept {
  return std::get<bool>(storage_);
}

double Value::as_number() const noexcept {
  return std::get<double>(storage_);
}

Object* Value::as_object() const noexcept {
  return std::get<Object*>(storage_);
}

bool Value::is_truthy() const noexcept {
  if (is_nil()) return false;
  if (is_boolean()) return as_boolean();
  return true;
}

bool Value::is_equal(const Value& other) const noexcept {
  if (is_nil() && other.is_nil()) return true;
  if (is_nil() || other.is_nil()) return false;

  if (storage_.index() != other.storage_.index()) return false;

  if (is_boolean()) return as_boolean() == other.as_boolean();
  if (is_number()) return as_number() == other.as_number();
  if (is_object()) return as_object() == other.as_object();

  return false;
}

str_t Value::stringify() const noexcept {
  if (is_nil()) return "nil";
  if (is_boolean()) return as_boolean() ? "true" : "false";
  if (is_number()) {
    double val = as_number();
    if (val == std::floor(val) && !std::isinf(val) && !std::isnan(val)) {
      return std::format("{}", static_cast<long long>(val));
    }
    return std::format("{:g}", val);
  }
  if (is_object()) {
    return as_object()->stringify();
  }
  return "nil";
}

// Convenience object type checks
bool Value::is_string() const noexcept { return is_obj_type(*this, ObjectType::OBJ_STRING); }
bool Value::is_function() const noexcept { return is_obj_type(*this, ObjectType::OBJ_FUNCTION); }
bool Value::is_closure() const noexcept { return is_obj_type(*this, ObjectType::OBJ_CLOSURE); }
bool Value::is_class() const noexcept { return is_obj_type(*this, ObjectType::OBJ_CLASS); }
bool Value::is_instance() const noexcept { return is_obj_type(*this, ObjectType::OBJ_INSTANCE); }

std::ostream& operator<<(std::ostream& os, const Value& value) {
  os << value.stringify();
  return os;
}

} // namespace ms
