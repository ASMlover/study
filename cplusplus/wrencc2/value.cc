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
#include "value.hh"

namespace wrencc {

StringObject* ObjValue::as_string() const noexcept {
  return nullptr;
}

const char* ObjValue::as_cstring() const noexcept {
  return nullptr;
}

ListObject* ObjValue::as_list() const noexcept {
  return nullptr;
}

RangeObject* ObjValue::as_range() const noexcept {
  return nullptr;
}

MapObject* ObjValue::as_map() const noexcept {
  return nullptr;
}

ModuleObject* ObjValue::as_module() const noexcept {
  return nullptr;
}

FunctionObject* ObjValue::as_function() const noexcept {
  return nullptr;
}

ForeignObject* ObjValue::as_foreign() const noexcept {
  return nullptr;
}

UpvalueObject* ObjValue::as_upvalue() const noexcept {
  return nullptr;
}

ClosureObject* ObjValue::as_closure() const noexcept {
  return nullptr;
}

FiberObject* ObjValue::as_fiber() const noexcept {
  return nullptr;
}

ClassObject* ObjValue::as_class() const noexcept {
  return nullptr;
}

InstanceObject* ObjValue::as_instance() const noexcept {
  return nullptr;
}

bool ObjValue::is_same(const ObjValue& r) const noexcept {
  return false;
}

bool ObjValue::is_equal(const ObjValue& r) const noexcept {
  return false;
}

u32_t ObjValue::hasher() const noexcept {
  return 0;
}

str_t ObjValue::stringify() const noexcept {
  return "";
}

}
