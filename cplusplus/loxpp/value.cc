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
#include "callable.h"
#include "instance.h"
#include "value.h"

inline std::string numeric_to_string(double d) {
  std::ostringstream oss;
  oss << d;
  return oss.str();
}

template <typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::string Value::stringify(void) const {
  return std::visit(overloaded {
        [](std::nullptr_t) -> std::string { return "nil"; },
        [](bool b) -> std::string { return b ? "true" : "false"; },
        [](double d) -> std::string { return numeric_to_string(d); },
        [](const std::string& s) -> std::string { return s; },
        [](const CallablePtr& c) -> std::string { return c->to_string(); },
        [](const InstancePtr& i) -> std::string { return i->to_string(); },
      }, v_);
}

bool Value::is_truthy(void) const {
  return std::visit(overloaded {
        [](std::nullptr_t) -> bool { return false; },
        [](bool b) -> bool { return b; },
        [](const auto&) -> bool { return true; },
      }, v_);
}
