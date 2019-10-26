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

using WrenForeignFn = std::function<void (WrenVM*)>;
using WrenFinalizeFn = std::function<void (void*)>;

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
  WrenFinalizeFn finalize;
};

// immediately run the garbage collector to free unused memory
void wrenCollectGarbage(WrenVM& vm);

// gets the numeric value of [value]
//
// it is an error to call this if the value is not a numeric
double wrenGetValueAsDouble(WrenVM& vm, WrenValue* value);

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

// ensures that the foreign method stack has at least [num_slots] available
// for use, growing the stack if needed
//
// does not shrink the stack if it has more than enough slots
//
// it is an error to call this from a finalizer
void wrenEnsureSlots(WrenVM& vm, int num_slots);

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

// stores the value captured in [value] in [slot]
//
// this does not release the handle for the value
void wrenSetSlotValue(WrenVM& vm, int slot, WrenValue* value);

// stores a new empty list in [slot]
void wrenSetSlotNewList(WrenVM& vm, int slot);

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

}
