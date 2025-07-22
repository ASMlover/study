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
#include "Value.hh"

namespace ms {

template <typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

str_t Value::name_of_type() const noexcept {
  return std::visit(overloaded {
        [](std::monostate) -> str_t { return "nil"; },
        [](bool) -> str_t { return "boolean"; },
        [](double) -> str_t { return "number"; },
        [](const str_t&) -> str_t { return "string"; },
        [](FunctionPtr) -> str_t { return "function"; },
        [](ClassPtr) -> str_t { return "class"; },
        [](InstancePtr) -> str_t { return "instance"; },
        [](ModulePtr) -> str_t { return "module"; },
      }, v_);
}

bool Value::is_truthy() const noexcept {
  return std::visit(overloaded {
        [](std::monostate) -> bool { return false; },
        [](bool b) -> bool { return b; },
        [](double d) -> bool { return d != 0.0; },
        [](const str_t& s) -> bool { return !s.empty(); },
        [](FunctionPtr) -> bool { return true; },
        [](ClassPtr) -> bool { return true; },
        [](InstancePtr) -> bool { return true; },
        [](ModulePtr) -> bool { return true; },
      }, v_);
}

str_t Value::stringify() const noexcept {
  return std::visit(overloaded {
        [](std::monostate) -> str_t { return "nil"; },
        [](bool b) -> str_t { return b ? "true" : "false"; },
        [](double d) -> str_t { return ms::as_string(d); },
        [](const str_t& s) -> str_t { return s; },
        [](FunctionPtr) -> str_t { return "<function>"; },
        [](ClassPtr) -> str_t { return "<class>"; },
        [](InstancePtr) -> str_t { return "<instance>"; },
        [](ModulePtr) -> str_t { return "<module>"; },
      }, v_);
}

}
