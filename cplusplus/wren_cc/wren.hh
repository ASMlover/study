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

#include <functional>
#include "common.hh"

namespace wrencc {

class WrenVM;

using WrenForeignFn = std::function<void (WrenVM&)>;

// defines a foreign method implemented by the host application, looks for a
// global class named [class_name] to bind the method, if not found, it will
// be created automatically
//
// defines a method on that class named [method_name] accepting [num_params]
// parameters, if a method already exists with that name and arity, it will
// be replaced, when invoked, the method will call [method]
void wrenDefineMethod(WrenVM& vm,
    const str_t& class_name, const str_t& method_name,
    int num_params, const WrenForeignFn& method);

// defines a static foreign method implemented by the host application, looks
// for a global class named [class_name] to bind the method to, if not found,
// it will be created automatically
//
// defines a static method on that class named [method_name] accepting
// [num_params] parameters, if a method already exists with that name and
// arity, it will be replaced, when invoked, the method will call [method]
void wrenDefineStaticMethod(WrenVM& vm,
    const str_t& class_name, const str_t& method_name,
    int num_params, const WrenForeignFn& method);

// reads an numeric argument for a foreign call, this must only be called wi-
// thin a function provided to [wrenDefineMethod] retrieves the argument at
// [index] with ranges from 0 to the number of parameters the method expects-1
double wrenGetArgumentDouble(WrenVM& vm, int index);

// reads a string argument for a foreign call, this must only be called within
// a function provided to [wrenDefineMethod], retrieves the argument at [index]
// which ranges from 0 to the number of parameters the method expects -1
//
// the memory for the returned string is owned by Wren. you can inspect it
// while in your foreign function, but cannot keep a pointer to it after the
// function returns, since the garbage collector may reclaim it
const char* wrenGetArgumentString(WrenVM& vm, int index);

// provides a numeric return value for a foreign call. this must only be called
// within a function provided to [wrenDefineMethod] once this is called, the
// foreign call is done and no more arguments can be read or return calls made
void wrenReturnDouble(WrenVM& vm, double value);

}
