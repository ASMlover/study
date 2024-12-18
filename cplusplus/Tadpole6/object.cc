// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include "object.hh"
#include "string_object.hh"
#include "native_object.hh"
#include "function_object.hh"
#include "upvalue_object.hh"
#include "closure_object.hh"

namespace tadpole {

const char* BaseObject::type_asstr() const noexcept {
  switch (type_) {
  case ObjType::STRING: return "<STRING>";
  case ObjType::NATIVE: return "<NATIVE>";
  case ObjType::FUNCTION: return "<FUNCTION>";
  case ObjType::UPVALUE: return "<UPVALUE>";
  case ObjType::CLOSURE: return "<CLOSURE>";
  default: break;
  }
  return "<UNKNOWN>";
}

StringObject* BaseObject::as_string() {
  return as_down<StringObject>(this);
}

const char* BaseObject::as_cstring() {
  return as_down<StringObject>(this)->cstr();
}

NativeObject* BaseObject::as_native() {
  return as_down<NativeObject>(this);
}

FunctionObject* BaseObject::as_function() {
  return as_down<FunctionObject>(this);
}

UpvalueObject* BaseObject::as_upvalue() {
  return as_down<UpvalueObject>(this);
}

ClosureObject* BaseObject::as_closure() {
  return as_down<ClosureObject>(this);
}

}
