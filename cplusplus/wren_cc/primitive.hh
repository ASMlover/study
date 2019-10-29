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
#pragma once

#include <tuple>
#include "value.hh"

namespace wrencc {

#define DEF_PRIMITIVE(fn)\
static bool _primitive_##fn(WrenVM& vm, Value* args)
#define RETURN_VAL(val) do {\
  args[0] = val;\
  return true;\
} while (false)
#define RETURN_ERR(msg) do {\
  vm.fiber()->set_error(StringObject::make_string(vm, msg));\
  return false;\
} while (false)
#define RETURN_FERR(fmt, arg) do {\
  vm.fiber()->set_error(StringObject::format(vm, fmt, arg));\
  return false;\
} while (false)

int validate_index(const Value& index, int count);
bool validate_function(WrenVM& vm, const Value& arg, const str_t& arg_name);
bool validate_numeric(WrenVM& vm, const Value& arg, const str_t& arg_name);
bool validate_int_value(WrenVM& vm, double value, const str_t& arg_name);
bool validate_int(WrenVM& vm, const Value& arg, const str_t& arg_name);
int validate_index(
    WrenVM& vm, const Value& arg, int count, const str_t& arg_name);
bool validate_string(WrenVM& vm, const Value& arg, const str_t& arg_name);
bool validate_key(WrenVM& vm, const Value& arg);
std::tuple<int, int, int> calculate_range(
    WrenVM& vm, RangeObject* range, int length);

}
