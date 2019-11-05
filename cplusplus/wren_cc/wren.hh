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
struct WrenHandle;

enum class WrenError {
  COMPILE, // a syntax or resolution error detected at compile time
  RUNTIME, // the error message for a runtime error
  SATCK_TRACE, // one entry of a runtime error's stack trace
};

using WrenForeignFn = std::function<void (WrenVM*)>;
using WrenFinalizeFn = std::function<void (void*)>;
using WrenErrorFn = std::function<void (WrenError, const str_t&, int, const str_t&)>;

enum class InterpretRet {
  SUCCESS,
  COMPILE_ERROR,
  RUNTIME_ERROR,
};

// the type of an object stored in a slot
//
// this is not necessarily the object's `class`, but instead its low level
// representation type
enum class WrenType {
  BOOLEAN,
  NUMERIC,
  FOREIGN,
  LIST,
  NIL,
  STRING,

  // the object is of a type that isn't accessible by the C++ API
  UNKNOWN,
};

struct WrenForeignClass {
  // the callback invoked when the foreign object is created
  //
  // this must be provided, inside the body of this, it must call
  // [wrenSetSlotNewForeign] exactly once
  WrenForeignFn allocate;

  // the callback invoked when the garbage collecator is about to collect a
  // foreign object's memory
  //
  // this may be `nullptr` if the foreign class does not need it finalize
  WrenFinalizeFn finalize;
};

// immediately run the garbage collector to free unused memory
void wrenCollectGarbage(WrenVM& vm);

// creates a handle that can be used to invoke a method with [signature] on
// using a receiver and arguments that are set up on the stack
//
// this handle can be used repreatedly to directly invoke that method from
// C++ code using [wrenCall]
//
// when you are done with this handle, it must be released using
// [wrenReleaseHandle]
WrenHandle* wrenMakeCallHandle(WrenVM& vm, const str_t& signature);

// [method] must have been created by a call to [wrenMakeCallHandle], the
// arguments to the method must be already on the stack, the receiver should
// be in slot 0 with the remaining arguments following it, in order, it is
// an error if the number of arguments provided does not match the method's
// signature
//
// after this returns, you can access the return value from slot 0 on the
// stack
InterpretRet wrenCall(WrenVM& vm, WrenHandle* method);

// releases the reference stored in [handle], after calling this, [handle]
// can no longer be used
void wrenReleaseHandle(WrenVM& vm, WrenHandle* handle);

// returns the number of slots available to the current foreign method
int wrenGetSlotCount(WrenVM& vm);

// ensures that the foreign method stack has at least [num_slots] available
// for use, growing the stack if needed
//
// does not shrink the stack if it has more than enough slots
//
// it is an error to call this from a finalizer
void wrenEnsureSlots(WrenVM& vm, int num_slots);

// gets the type of the object int [slot]
WrenType wrenGetSlotType(WrenVM& vm, int slot);

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
// until the handle is released by calling [wrenReleaseHandle()]
WrenHandle* wrenGetSlotHandle(WrenVM& vm, int slot);

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

// stores the boolean [value] in [slot]
void wrenSetSlotBool(WrenVM& vm, int slot, bool value);

// stores the numeric [value] in [slot]
void wrenSetSlotDouble(WrenVM& vm, int slot, double value);

// stores nil in [slot]
void wrenSetSlotNil(WrenVM& vm, int slot);

// stores the string in [slot]
//
// the [text] is copied to a new string within Wren's heap, so you can free
// memory used by it after this is called.
void wrenSetSlotString(WrenVM& vm, int slot, const str_t& text);

// stores the value captured in [handle] in [slot]
//
// this does not release the handle for the value
void wrenSetSlotHandle(WrenVM& vm, int slot, WrenHandle* handle);

// stores a new empty list in [slot]
void wrenSetSlotNewList(WrenVM& vm, int slot);

// creates a new instance of the foreign class stored in [class_slot] with
// [size] bytes of raw storage and places the resulting object in [slot]
//
// this does not invoke the foreign class's constructor on the new instance,
// if you need that to happen, call the constructor from Wren, which will
// then call the allocator foreign method, in there, call this to create the
// object and then the constructor will be invoked when the allocator returns
//
// returns a pointer to the foreign object's data
void* wrenSetSlotNewForeign(WrenVM& vm, int slot, int class_slot, sz_t size);

// returns the number of elements in the list stored in [slot]
int wrenGetListCount(WrenVM& vm, int slot);

// reads element [index] from the list in [list_slot] and stores it in
// [element_slot]
void wrenGetListElement(WrenVM& vm, int list_slot, int index, int element_slot);

// takes the value stored at [element_slot] and inserts it into the list stored
// at [list_slot] at [index]
//
// as in Wren, negative indexes can be used to insert from the end, to append
// at element, use `-1` for the index
void wrenInsertInList(WrenVM& vm, int list_slot, int index, int element_slot);

// looks up the top level variable with [name] in [module] and stores it in
// [to_slot]
void wrenGetVariable(WrenVM& vm,
    const str_t& module, const str_t& name, int to_slot);

// sets the current fiber to be aborted, and uses the value in [slot] as the
// runtime error object
void wrenAbortFiber(WrenVM& vm, int slot);

}
