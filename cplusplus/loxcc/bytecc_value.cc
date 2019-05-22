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
#include "bytecc_value.hh"

namespace loxcc::bytecc {

StringObject* Value::as_string(void) const {
  return nullptr;
}

const char* Value::as_cstring(void) const {
  return nullptr;
}

Value::NativeTp Value::as_native(void) const {
  return nullptr;
}

FunctionObject* Value::as_function(void) const {
  return nullptr;
}

UpvalueObject* Value::as_upvalue(void) const {
  return nullptr;
}

ClosureObject* Value::as_closure(void) const {
  return nullptr;
}

ClassObject* Value::as_class(void) const {
  return nullptr;
}

InstanceObject* Value::as_instance(void) const {
  return nullptr;
}

BoundMehtodObject* Value::as_bound_method(void) const {
  return nullptr;
}

bool Value::operator==(const Value& r) const {
  if (this == &r)
    return true;
  if (type_ != r.type_)
    return false;

  switch (type_) {
  case ValueType::NIL: return true;
  case ValueType::BOOLEAN: return as_.boolean == r.as_.boolean;
  case ValueType::NUMERIC: return as_.numeric == r.as_.numeric;
  case ValueType::OBJECT: return as_.object == r.as_.object; // TODO: FIXME:
  }
  return false;
}

bool Value::operator!=(const Value& r) const {
  return !(*this == r);
}

bool Value::is_truthy(void) const {
  switch (type_) {
  case ValueType::NIL: return false;
  case ValueType::BOOLEAN: return as_.boolean;
  case ValueType::NUMERIC: return as_.numeric == 0.f;
  case ValueType::OBJECT: return as_.object->is_truthy();
  }
  return false;
}

str_t Value::stringify(void) const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::BOOLEAN: return as_.boolean ? "true" : "false";
  case ValueType::NUMERIC: return Xt::to_string(as_.numeric);
  case ValueType::OBJECT: return as_.object->stringify();
  }
  return "";
}

}
