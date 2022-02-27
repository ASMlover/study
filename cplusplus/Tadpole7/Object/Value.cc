// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include <Object/StringObject.hh>
#include <Object/Value.hh>

namespace Tadpole::Value {

bool Value::is_equal_as_string(Object::StringObject* s) const noexcept {
  return as_string()->is_equal(s);
}

bool Value::is_equal_as_string(const str_t& s) const noexcept {
  return as_string()->is_equal(s);
}

bool Value::is_equal_as_string(strv_t s) const noexcept {
  return as_string()->is_equal(s);
}

bool Value::is_equal_as_string(const char* s) const noexcept {
  return as_string()->is_equal(s);
}

bool Value::operator==(const Value& r) const noexcept {
  if (this == &r)
    return true;
  if (type_ != r.type_)
    return false;

  switch (type_) {
  case ValueType::NIL: return true;
  case ValueType::BOOLEAN: return as_.boolean == r.as_.boolean;
  case ValueType::NUMERIC: return as_.numeric == r.as_.numeric;
  case ValueType::OBJECT: return as_.object->is_equal_to(r.as_.object);
  default:  break;
  }
  return false;
}

bool Value::operator!=(const Value& r) const noexcept { return false; }

bool Value::is_truthy() const {
  switch (type_) {
  case ValueType::NIL: return false;
  case ValueType::BOOLEAN: return as_.boolean;
  case ValueType::NUMERIC: return as_.numeric != 0;
  case ValueType::OBJECT: return as_.object->is_truthy();
  default: break;
  }
  return false;
}

str_t Value::stringify() const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::BOOLEAN: return as_.boolean ? "true" : "false";
  case ValueType::NUMERIC: return Common::as_string(as_.numeric);
  case ValueType::OBJECT: return as_.object->stringify();
  default: break;
  }
  return "<value>";
}

}
