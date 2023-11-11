// Copyright (c) 2023 ASMlover. All rights reserved.
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
#include "value.hh"
#include "object.hh"

namespace clox {

bool Value::is_obj_type(ObjType type) const noexcept {
  return is_obj() && as_obj()->is_type(type);
}

ObjString* Value::as_string() const noexcept { return as_.obj->as_string(); }
cstr_t Value::as_cstring() const noexcept { return as_.obj->as_cstring(); }
ObjFunction* Value::as_function() const noexcept { return as_.obj->as_function(); }

bool Value::is_equal(const Value& x) const noexcept {
  if (type_ != x.type_)
    return false;

  switch (type_) {
  case ValueType::VAL_BOOL: return as_.boolean == x.as_.boolean;
  case ValueType::VAL_NIL: return true;
  case ValueType::VAL_NUMBER: return as_.number == x.as_.number;
  case ValueType::VAL_OBJ: return as_.obj->is_equal(x.as_.obj);
  default: return false; // unreachable
  }
  return false;
}

str_t Value::stringify() const {
  switch (type_) {
  case ValueType::VAL_BOOL: return as_.boolean ? "true" : "false";
  case ValueType::VAL_NIL: return "nil";
  case ValueType::VAL_NUMBER: return clox::as_string(as_.number);
  case ValueType::VAL_OBJ: return as_.obj->stringify();
  }
  return "<value>";
}

}
