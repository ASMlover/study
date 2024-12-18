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
#include <sstream>
#include "value.h"

namespace lox {

template <typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static std::string decimal2str(double d) {
  std::ostringstream oss;
  oss << d;
  return oss.str();
}

bool Value::is_truthy(void) const {
  return std::visit(overloaded {
        [](std::nullptr_t) -> bool { return false; },
        [](bool b) -> bool { return b; },
        [](std::int64_t i64) -> bool { return i64 != 0; },
        [](double d) -> bool { return d != 0.f; },
        [](const std::string& s) -> bool { return !s.empty(); },
      }, v_);
}

std::string Value::stringify(void) const {
  return std::visit(overloaded {
        [](std::nullptr_t) -> std::string { return "nil"; },
        [](bool b) -> std::string { return b ? "true" : "false"; },
        [](std::int64_t i64) -> std::string { return std::to_string(i64); },
        [](double d) -> std::string { return decimal2str(d); },
        [](const std::string& s) -> std::string { return "\"" + s + "\""; },
      }, v_);
}

std::string Value::type(void) const {
  return std::visit(overloaded {
        [](std::nullptr_t) -> std::string { return "nil"; },
        [](bool) -> std::string { return "boolean"; },
        [](std::int64_t) -> std::string { return "integer"; },
        [](double) -> std::string { return "decimal"; },
        [](const std::string&) -> std::string { return "string"; },
      }, v_);
}

std::ostream& operator<<(std::ostream& out, const Value& r) {
  return out << r.stringify();
}

}
