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
struct WrenValue;

using WrenForeignFn = std::function<void (WrenVM&)>;

enum class InterpretRet {
  SUCCESS,
  COMPILE_ERROR,
  RUNTIME_ERROR,
};

struct WrenForeignClass {
  // the callback invoked when the foreign object is created
  //
  // this must be provided, inside the body of this, it must call
  // [wrenAllocateForeign] exactly once
  WrenForeignFn allocate;

  // the callback invoked when the garbage collecator is about to collect a
  // foreign object's memory
  //
  // this may be `nullptr` if the foreign class does not need it finalize
  WrenForeignFn finalize;
};

// immediately run the garbage collector to free unused memory
void wrenCollectGarbage(WrenVM& vm);

// releases the reference stored in [value], after calling this, [value] can no
// longer be used
void wrenReleaseValue(WrenVM& vm, WrenValue* value);

// this must be called once inside a foreign class's allocator function
//
// it tells When how many bytes of raw data need to be stored in the foreign
// object and creates the new object with that size, it returns a pointer to
// the foreign object's data
void* wrenAllocateForeign(WrenVM& vm, sz_t size);

// returns the number of slots available to the current foreign method
int wrenGetSlotCount(WrenVM& vm);

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

// reads a boolean value from [slot]
//
// it is an error to call this if the slot does not contain a boolean value
bool wrenGetSlotBool(WrenVM& vm, int slot);

// reads a numeric value from [slot]
//
// it is an error to call this if the slot does not contain a numeric value
double wrenGetSlotDouble(WrenVM& vm, int slot);

// reads a string from [slot]
//
// the memory for the returned string is owned by Wren. you can inspect it
// while in your foreign method, but cannot keep a pointer to it after the
// function returns, since the garbage collector may reclaim it
//
// it is an error to call this if the slot does not contain a string
const char* wrenGetSlotString(WrenVM& vm, int slot);

// creates a handle for the value stored in [slot]
//
// this will prevent the object that is referred to from being garbage collected
// until the handle is released by calling [wrenReleaseValue()]
WrenValue* wrenGetSlotValue(WrenVM& vm, int slot);

// reads a foreign object from [slot] and returns a pointer to the foreign data
// stored with it
//
// it is an error to call this if the slot does not contain an instance of a
// foreign class
void* wrenGetSlotForeign(WrenVM& vm, int slot);

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

// provides the return value for a foreign call
//
// this uses the value referred to by the handle as the return value, but it
// does not release the handle
void wrenReturnValue(WrenVM& vm, WrenValue* value);

}
