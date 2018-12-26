// Copyright (c) 2018 ASMlover. All rights reserved.
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

#include <string>
#include <variant>

class Value {
  std::variant<std::nullptr_t, bool, double, std::string> t_;
public:
  Value(void)
    : t_(nullptr) {
  }

  Value(bool v)
    : t_(v) {
  }

  Value(double v)
    : t_(v) {
  }

  Value(const std::string& v)
    : t_(v) {
  }

  bool is_null(void) const {
    return std::holds_alternative<std::nullptr_t>(t_);
  }

  bool is_boolean(void) const {
    return std::holds_alternative<bool>(t_);
  }

  bool is_numeric(void) const {
    return std::holds_alternative<double>(t_);
  }

  bool is_string(void) const {
    return std::holds_alternative<std::string>(t_);
  }

  bool to_boolean(void) const {
    return std::get<bool>(t_);
  }

  double to_numeric(void) const {
    return std::get<double>(t_);
  }

  std::string to_string(void) const {
    return std::get<std::string>(t_);
  }

  std::string stringify(void) const;
};
