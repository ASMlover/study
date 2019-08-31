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
// defines a method on that class named [method_name] accepting [arity]
// parameters, if a method already exists with that name and arity, it will
// be replaced, when invoked, the method will call [method]
void wrenDefineMethod(WrenVM& vm,
    const str_t& class_name, const str_t& method_name,
    int arity, const WrenForeignFn& method);

// defines a static foreign method implemented by the host application, looks
// for a global class named [class_name] to bind the method to, if not found,
// it will be created automatically
//
// defines a static method on that class named [method_name] accepting
// [arity] parameters, if a method already exists with that name and
// arity, it will be replaced, when invoked, the method will call [method]
void wrenDefineStaticMethod(WrenVM& vm,
    const str_t& class_name, const str_t& method_name,
    int arity, const WrenForeignFn& method);

// the following functions read one of the arguments passed to a foreign call.
// they may only be called while within a function provided to
// [wrenDefineMethod] or [wrenDefineStaticMethod] that Wren has invoked.
//
// they retreive the argument at a given index which ranges from 0 to the number
// of parameters the method expects, the zeroth parameter is used for the
// receiver of the method, for example, given a foreign method `foo` on String
// invoked like:
//
//    "receiver".foo("one", "two", "three")
//
// the foreign function will be able to access the arguments like so:
//
//    0: "receiver"
//    1: "one"
//    2: "two"
//    3: "three"
//
// it is an error to pass an invalid argument index

// reads a boolean argument for a foreign call, returns false if the argument
// is not a boolean
bool wrenGetArgumentBool(WrenVM& vm, int index);

// reads a numeric argument for a foreign call, returns 0 if the argument is
// not a numeric value
double wrenGetArgumentDouble(WrenVM& vm, int index);

// reads an string argument for a foreign call, returns nullptr if the argument
// is not a string
//
// the memory for the returned string is owned by Wren. you can inspect it
// while in your foreign function, but cannot keep a pointer to it after the
// function returns, since the garbage collector may reclaim it
const char* wrenGetArgumentString(WrenVM& vm, int index);

// the following functions provide the return value for a foreign method back
// to Wren, like above they may only be called during a foreign call invoked
// by Wren
//
// if one of these is called by the time the foreign funtion returns, the method
// implicitly returns `nil`, within a given foreign call, you may only call one
// of these once, it is an error to access any of the foreign calls arguments
// after one of these has been called.

// provides a boolean return value for a foreign call
void wrenReturnBool(WrenVM& vm, bool value);

// provides a numeric return value for a foreign call
void wrenReturnDouble(WrenVM& vm, double value);

// provides a string return value for a foreign call
//
// the [text] will be copied to a new string within Wren's heap, so you can
// free memory used by it after this is called, if [text] is non-empty, Wren
// will copy that from [text]
void wrenReturnString(WrenVM& vm, const str_t& text);

}
