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
#include "value.hh"

namespace lvm {

static inline std::string numeric_as_string(double d) {
  std::ostringstream oss;
  oss << d;
  return oss.str();
}

bool Value::is_equal(const Value& r) const {
  if (type_ != r.type_)
    return false;

  switch (type_) {
  case ValueType::NIL: return true;
  case ValueType::BOOLEAN: return as_boolean() == r.as_boolean();
  case ValueType::NUMERIC: return as_numeric() == r.as_numeric();
  default: break;
  }
  return false;
}

bool Value::is_truthy(void) const {
  switch (type_) {
  case ValueType::NIL: return false;
  case ValueType::BOOLEAN: return as_.boolean;
  case ValueType::NUMERIC: return as_.numeric != 0;
  default: break;
  }
  return false;
}

std::string Value::stringify(void) const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::BOOLEAN: return as_.boolean ? "true" : "false";
  case ValueType::NUMERIC: return numeric_as_string(as_.numeric);
  default: break;
  }
  return "";
}

std::ostream& operator<<(std::ostream& out, const Value& value) {
  return out << value.stringify();
}

}
