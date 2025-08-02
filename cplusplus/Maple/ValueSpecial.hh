// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include "Common.hh"
#include "ValueHelper.hh"

namespace ms {

inline ValueType get_value_type(const ValueSpecial& value) noexcept {
  return as_type<ValueType>(value.index());
}

inline str_t value_to_string(const ValueSpecial& value) noexcept {
  switch (get_value_type(value)) {
  case ValueType::NIL: return "nil";
  case ValueType::BOOLEAN: return std::get<bool>(value) ? "true" : "false";
  case ValueType::NUMBER: return ms::as_string(std::get<double>(value));
  case ValueType::STRING: return std::get<str_t>(value);
  case ValueType::FUNCTION: return "<function>";
  case ValueType::CLASS: return "<class>";
  case ValueType::INSTANCE: return "<instance>";
  case ValueType::MODULE: return "<module>";
  default: break;
  }
  return "<unknown>";
}

inline bool is_truthy(const ValueSpecial& value) noexcept {
  switch (get_value_type(value)) {
  case ValueType::NIL: return false;
  case ValueType::BOOLEAN: return std::get<bool>(value);
  case ValueType::NUMBER: return std::get<double>(value) != 0.0;
  case ValueType::STRING: return !std::get<str_t>(value).empty();
  case ValueType::FUNCTION: return true;
  case ValueType::CLASS: return true;
  case ValueType::INSTANCE: return true;
  case ValueType::MODULE: return true;
  default: break;
  }
  return false;
}

}
