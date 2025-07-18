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

#include "Common.hh"

namespace ms {

enum class ValueType : u8_t {
  NIL,
  BOOL,
  INT,
  FLOAT,
  STRING,
  FUNCTION,
  NATIVE_FUNCTION,
  MODULE
};

class Environment;
class Function;
class Module;

class Value final : public Copyable {
  ValueType type_{ValueType::NIL};
  union {
    bool      boolean;
    i64_t     integer;
    double    floating;
    str_t*    str;
    Function* function;
    struct {
      void*   func_ptr;
      cstr_t  name;
    } native_function;
    Module*   module;
  } as_;

  template <typename T> inline void set_integer(T x) noexcept { as_.integer = as_type<i64_t>(x); }
  template <typename T> inline void set_numeric(T x) noexcept { as_.floating = as_type<double>(x); }
public:
  Value() noexcept {}
  Value(nil_t) noexcept {}
  Value(bool b) noexcept : type_{ValueType::BOOL} { as_.boolean = b; }
  Value(i8_t i) noexcept : type_{ValueType::INT} { set_integer(i); }
  Value(u8_t i) noexcept : type_{ValueType::INT} { set_integer(i); }
  Value(i16_t i) noexcept : type_{ValueType::INT} { set_integer(i); }
  Value(u16_t i) noexcept : type_{ValueType::INT} { set_integer(i); }
  Value(i32_t i) noexcept : type_{ValueType::INT} { set_integer(i); }
  Value(u32_t i) noexcept : type_{ValueType::INT} { set_integer(i); }
  Value(i64_t i) noexcept : type_{ValueType::INT} { as_.integer = i; }
  Value(u64_t i) noexcept : type_{ValueType::INT} { set_integer(i); }
#if defined(MAPLE_GNUC)
  Value(long long ll) noexcept : type_{ValueType::INT} { set_integer(ll); }
  Value(unsigned long long ull) noexcept : type_{ValueType::INT} { set_integer(ull); }
#endif
  Value(float f) noexcept : type_{ValueType::FLOAT} { set_numeric(f); }
  Value(double d) noexcept : type_{ValueType::FLOAT} { as_.floating = d; }
  Value(const str_t& s) noexcept : type_{ValueType::STRING} { as_.str = make_string(s); }
  Value(strv_t s) noexcept : type_{ValueType::STRING} { as_.str = make_string(s); }
  Value(cstr_t s) noexcept : type_{ValueType::STRING} { as_.str = make_string(s); }
  Value(Function* f) noexcept : type_{ValueType::FUNCTION} { as_.function = f; }
  Value(void* func, cstr_t name) noexcept : type_{ValueType::NATIVE_FUNCTION} {
    as_.native_function.func_ptr = func;
    as_.native_function.name = name;
  }
  Value(Module* m) : type_{ValueType::MODULE} { as_.module = m; }

  ~Value() noexcept {
    if (type_ == ValueType::STRING)
      free_string(as_.str);
  }
};

}
