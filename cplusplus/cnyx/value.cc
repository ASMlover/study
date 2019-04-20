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
#include <sstream>
#include "object.hh"
#include "value.hh"

namespace nyx {

inline str_t numeric_as_string(double d) {
  std::stringstream ss;
  ss << d;
  return ss.str();
}

bool operator==(const Value& a, const Value& b) {
  if (a.type() != b.type())
    return false;

  switch (a.type()) {
  case ValueType::NIL: return true;
  case ValueType::BOOLEAN: return a.as_boolean() == b.as_boolean();
  case ValueType::NUMERIC: return a.as_numeric() == b.as_numeric();
  case ValueType::OBJECT: return a.as_object() == b.as_object();
  default: break;
  }
  return false;
}

bool operator!=(const Value& a, const Value& b) {
  return !(a == b);
}

bool Value::is_string(void) const noexcept {
  return is_obj_type(*this, ObjType::STRING);
}

bool Value::is_closure(void) const noexcept {
  return is_obj_type(*this, ObjType::CLOSURE);
}

bool Value::is_function(void) const noexcept {
  return is_obj_type(*this, ObjType::FUNCTION);
}

bool Value::is_native(void) const noexcept {
  return is_obj_type(*this, ObjType::NATIVE);
}

bool Value::is_upvalue(void) const noexcept {
  return is_obj_type(*this, ObjType::UPVALUE);
}

bool Value::is_class(void) const noexcept {
  return is_obj_type(*this, ObjType::CLASS);
}

bool Value::is_instance(void) const noexcept {
  return is_obj_type(*this, ObjType::INSTANCE);
}

bool Value::is_bound_method(void) const noexcept {
  return is_obj_type(*this, ObjType::BOUND_METHOD);
}

StringObject* Value::as_string(void) const noexcept {
  return Xptr::down<StringObject>(as_.obj);
}

const char* Value::as_cstring(void) const noexcept {
  return as_string()->chars();
}

ClosureObject* Value::as_closure(void) const noexcept {
  return Xptr::down<ClosureObject>(as_.obj);
}

FunctionObject* Value::as_function(void) const noexcept {
  return Xptr::down<FunctionObject>(as_.obj);
}

NativeFunction Value::as_native(void) const noexcept {
  return Xptr::down<NativeObject>(as_.obj)->get_function();
}

ClassObject* Value::as_class(void) const noexcept {
  return Xptr::down<ClassObject>(as_.obj);
}

InstanceObject* Value::as_instance(void) const noexcept {
  return Xptr::down<InstanceObject>(as_.obj);
}

BoundMethodObject* Value::as_bound_method(void) const noexcept {
  return Xptr::down<BoundMethodObject>(as_.obj);
}

str_t Value::stringify(void) const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::BOOLEAN: return as_.boolean ? "true" : "false";
  case ValueType::NUMERIC: return numeric_as_string(as_.numeric);
  case ValueType::OBJECT: return as_.obj->stringify();
  default: break;
  }
  return "";
}

std::ostream& operator<<(std::ostream& out, const Value& v) {
  return out << v.stringify();
}

}
