// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include "wren.hh"
#include "vm.hh"

namespace wrencc {

void wrenCollectGarbage(WrenVM& vm) {
  // TODO:
}

InterpretRet wrenInterpret(WrenVM& vm, const str_t& module, const str_t& source_bytes) {
  // TODO:
  return InterpretRet::SUCCESS;
}

WrenHandle* wrenMakeCallHandle(WrenVM& vm, const str_t& signature) {
  // TODO:
  return nullptr;
}

void wrenReleaseHandle(WrenVM& vm, WrenHandle* handle) {
  // TODO:
}

InterpretRet wrenCall(WrenVM& vm, WrenHandle* method) {
  // TODO:
  return InterpretRet::SUCCESS;
}

sz_t wrenGetSlotCount(WrenVM& vm) {
  // TODO:
  return 0;
}

void wrenEnsureSlots(WrenVM& vm, sz_t num_slots) {
  // TODO:
}

WrenType wrenGetSlotType(WrenVM& vm, sz_t slot) {
  // TODO:
  return WrenType::UNKNOWN;
}

bool wrenGetSlotBool(WrenVM& vm, sz_t slot) {
  // TODO:
  return false;
}

double wrenGetSlotDouble(WrenVM& vm, sz_t slot) {
  // TODO:
  return 0;
}

const char* wrenGetSlotString(WrenVM& vm, sz_t slot) {
  // TODO:
  return nullptr;
}

WrenHandle* wrenGetSlotHandle(WrenVM& vm, sz_t slot) {
  // TODO:
  return nullptr;
}

void* wrenGetSlotForeign(WrenVM& vm, sz_t slot) {
  // TODO:
  return nullptr;
}

void wrenSetSlotNil(WrenVM& vm, sz_t slot) {
  // TODO:
}

void wrenSetSlotBool(WrenVM& vm, sz_t slot, bool value) {
  // TODO:
}

void wrenSetSlotDouble(WrenVM& vm, sz_t slot, double value) {
  // TODO:
}

void wrenSetSlotString(WrenVM& vm, sz_t slot, const str_t& text) {
  // TODO:
}

void wrenSetSlotHandle(WrenVM& vm, sz_t slot, WrenHandle* handle) {
  // TODO:
}

void* wrenSetSlotNewForeign(WrenVM& vm, sz_t slot, sz_t class_slot, sz_t size) {
  // TODO:
  return nullptr;
}

void wrenSetSlotNewList(WrenVM& vm, sz_t slot) {
  // TODO:
}

sz_t wrenGetListCount(WrenVM& vm, sz_t slot) {
  // TODO:
  return 0;
}

void wrenGetListElement(WrenVM& vm, sz_t list_slot, int index, sz_t element_slot) {
  // TODO:
}

void wrenInsertInList(WrenVM& vm, sz_t list_slot, int index, sz_t element_slot) {
  // TODO:
}

void wrenGetVariable(WrenVM& vm, const str_t& module, const str_t& name, sz_t to_slot) {
  // TODO:
}

void wrenAbortFiber(WrenVM& vm, sz_t slot) {
  // TODO:
}

void* wrenGetUserData(WrenVM& vm) {
  // TODO:
  return nullptr;
}

void wrenSetUserData(WrenVM& vm, void* user_data) {
  // TODO:
}

}
