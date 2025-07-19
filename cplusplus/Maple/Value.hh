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

namespace ms {

enum class ValueType : u8_t {
  NIL,
  BOOLEAN,
  NUMBER,
  STRING,
  FUNCTION,
  CLASS,
  INSTANCE,
  MODULE
};

class Function;
class Class;
class Instance;
class Module;

using FunctionPtr = std::shared_ptr<Function>;
using ClassPtr    = std::shared_ptr<Class>;
using InstancePtr = std::shared_ptr<Instance>;
using ModulePtr   = std::shared_ptr<Module>;

class Value final : public Copyable {
  using VariantType = std::variant<
    nil_t,
    bool,
    double,
    std::string,
    FunctionPtr,
    ClassPtr,
    InstancePtr,
    ModulePtr>;

  VariantType v_{};

  inline ValueType value_type() const noexcept { return as_type<ValueType>(v_.index()); }

  template <typename T> inline void set_number(T x) noexcept { v_ = as_type<double>(x); }
  template <typename T> inline void set_string(T x) noexcept { v_ = str_t{x}; }
public:
  Value() noexcept : v_{nullptr} {}
  Value(nil_t) noexcept : v_{nullptr} {}
  Value(bool b) noexcept : v_{b} {}
  Value(i8_t i) noexcept { set_number(i); }
  Value(u8_t i) noexcept { set_number(i); }
  Value(i16_t i) noexcept { set_number(i); }
  Value(u16_t i) noexcept { set_number(i); }
  Value(i32_t i) noexcept { set_number(i); }
  Value(u32_t i) noexcept { set_number(i); }
  Value(i64_t i) noexcept { set_number(i); }
  Value(u64_t i) noexcept { set_number(i); }
#if defined(MAPLE_GNUC)
  Value(long long ll) noexcept { set_number(ll); }
  Value(unsigned long long ull) noexcept { set_number(ull); }
#endif
  Value(float f) noexcept { set_number(f); }
  Value(double d) noexcept : v_{d} {}
  Value(const str_t& s) noexcept : v_{s} {}
  Value(strv_t s) noexcept { set_string(s); }
  Value(cstr_t s) noexcept { set_string(s); }
  Value(const FunctionPtr& f) noexcept : v_{f} {}
  Value(const ClassPtr& c) noexcept : v_{c} {}
  Value(const InstancePtr& i) noexcept : v_{i} {}
  Value(const ModulePtr& m) noexcept : v_{m} {}
};

}
