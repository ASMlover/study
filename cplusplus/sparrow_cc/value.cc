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
#include "base_object.hh"
#include "class_object.hh"
#include "module_object.hh"
#include "string_object.hh"
#include "upvalue_object.hh"
#include "function_object.hh"
#include "closure_object.hh"
#include "value.hh"

namespace sparrow {

inline str_t numeric_to_string(double d) {
  std::stringstream ss;
  ss << d;
  return ss.str();
}

std::ostream& operator<<(std::ostream& out, const Value& value) {
  return out << value.stringify();
}

bool Value::operator==(const Value& r) const {
  if (type_ != r.type_)
    return false;

  switch (type_) {
  case ValueType::NIL: case ValueType::TRUE: case ValueType::FALSE: return true;
  case ValueType::NUMERIC: return as_.numeric == r.as_.numeric;
  case ValueType::OBJECT: return as_.object == r.as_.object;
  default: break;
  }
  return false;
}

bool Value::operator!=(const Value& r) const {
  return !(*this == r);
}

bool Value::is_truthy(void) const {
  switch (type_) {
  case ValueType::NIL: return false;
  case ValueType::TRUE: return true;
  case ValueType::FALSE: return false;
  case ValueType::NUMERIC: return as_.numeric != 0;
  case ValueType::OBJECT: return true;
  default: break;
  }
  return false;
}

str_t Value::stringify(void) const {
  switch (type_) {
  case ValueType::UNKNOWN: return "unknown";
  case ValueType::NIL: return "nil";
  case ValueType::TRUE: return "true";
  case ValueType::FALSE: return "false";
  case ValueType::NUMERIC: return numeric_to_string(as_.numeric);
  case ValueType::OBJECT: return "<object>";
  default: break;
  }
  return "";
}

ClassObject* Value::as_class(void) const {
  return Xt::down<ClassObject>(as_.object);
}

ModuleObject* Value::as_module(void) const {
  return Xt::down<ModuleObject>(as_.object);
}

StringObject* Value::as_string(void) const {
  return Xt::down<StringObject>(as_.object);
}

const char* Value::as_cstring(void) const {
  return Xt::down<StringObject>(as_.object)->c_str();
}

UpvalueObject* Value::as_upvalue(void) const {
  return Xt::down<UpvalueObject>(as_.object);
}

FunctionObject* Value::as_function(void) const {
  return Xt::down<FunctionObject>(as_.object);
}

ClosureObject* Value::as_closure(void) const {
  return Xt::down<ClosureObject>(as_.object);
}

}
