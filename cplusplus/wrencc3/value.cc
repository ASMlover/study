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
#include "value.hh"

namespace wrencc {

StringObject* BaseObject::as_string() noexcept {
  // TODO:
  return nullptr;
}

const char* BaseObject::as_cstring() noexcept {
  // TODO:
  return nullptr;
}

ListObject* BaseObject::as_list() noexcept {
  // TODO:
  return nullptr;
}

RangeObject* BaseObject::as_range() noexcept {
  // TODO:
  return nullptr;
}

MapObject* BaseObject::as_map() noexcept {
  // TODO:
  return nullptr;
}

ModuleObject* BaseObject::as_module() noexcept {
  // TODO:
  return nullptr;
}

FunctionObject* BaseObject::as_function() noexcept {
  // TODO:
  return nullptr;
}

ForeignObject* BaseObject::as_foreign() noexcept {
  // TODO:
  return nullptr;
}

UpvalueObject* BaseObject::as_upvalue() noexcept {
  // TODO:
  return nullptr;
}

ClosureObject* BaseObject::as_closure() noexcept {
  // TODO:
  return nullptr;
}

FiberObject* BaseObject::as_fiber() noexcept {
  // TODO:
  return nullptr;
}

ClassObject* BaseObject::as_class() noexcept {
  // TODO:
  return nullptr;
}

InstanceObject* BaseObject::as_instance() noexcept {
  // TODO:
  return nullptr;
}

bool ObjValue::is_same(const ObjValue& r) const noexcept {
  if (type_ != r.type_)
    return false;

  if (type_ == ValueType::NUMERIC)
    return as_.num == r.as_.num;
  return as_.obj == r.as_.obj;
}

bool ObjValue::is_equal(const ObjValue& r) const noexcept {
  if (is_same(r))
    return true;

  if (!is_object() || !r.is_object() || objtype() != r.objtype())
    return false;
  return as_.obj->is_equal(r.as_.obj);
}

u32_t ObjValue::hasher() const noexcept {
  switch (type_) {
  case ValueType::NIL: return 1;
  case ValueType::TRUE: return 2;
  case ValueType::FALSE: return 0;
  case ValueType::NUMERIC: return Xt::hash_numeric(as_.num);
  case ValueType::OBJECT: return as_.obj->hasher();
  default: UNREACHABLE();
  }
  return 0;
}

str_t ObjValue::stringify() const noexcept {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::TRUE: return "true";
  case ValueType::FALSE: return "false";
  case ValueType::NUMERIC: return Xt::to_string(as_.num);
  case ValueType::OBJECT: return as_.obj->stringify();
  default: UNREACHABLE();
  }
  return "";
}

}
