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

StringObject* BaseObject::as_string() noexcept {
  return Xt::down<StringObject>(this);
}

const char* BaseObject::as_cstring() noexcept {
  return Xt::down<StringObject>(this)->cstr();
}

ListObject* BaseObject::as_list() noexcept {
  return nullptr;
}

RangeObject* BaseObject::as_range() noexcept {
  return nullptr;
}

MapObject* BaseObject::as_map() noexcept {
  return nullptr;
}

ModuleObject* BaseObject::as_module() noexcept {
  return nullptr;
}

FunctionObject* BaseObject::as_function() noexcept {
  return nullptr;
}

ForeignObject* BaseObject::as_foreign() noexcept {
  return nullptr;
}

UpvalueObject* BaseObject::as_upvalue() noexcept {
  return nullptr;
}

ClosureObject* BaseObject::as_closure() noexcept {
  return nullptr;
}

FiberObject* BaseObject::as_fiber() noexcept {
  return nullptr;
}

ClassObject* BaseObject::as_class() noexcept {
  return nullptr;
}

InstanceObject* BaseObject::as_instance() noexcept {
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

  if (!is_object() || !r.is_object())
    return false;
  if (as_.obj->type() != r.as_.obj->type())
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

StringObject::StringObject(ClassObject* cls, char c) noexcept
  : BaseObject(ObjType::STRING, cls)
  , size_(1) {
  data_ = new char[size_ + 1];
  data_[0] = c, data_[1] = 0;

  hash_string();
}

StringObject::StringObject(
    ClassObject* cls, const char* s, u32_t n, bool replace_owner) noexcept
  : BaseObject(ObjType::STRING, cls)
  , size_(n) {
  if (replace_owner) {
    data_ = const_cast<char*>(s);
  }
  else {
    if (s != nullptr) {
      data_ = new char[size_ + 1];
      std::memcpy(data_, s, n);
      data_[size_] = 0;
    }
  }

  hash_string();
}

StringObject::~StringObject() noexcept {
  if (data_)
    delete [] data_;
}

void StringObject::hash_string() {
  // FNV-1a hash. See: http://www.isthe.com/chongo/tech/comp/fnv/
  u32_t hash = 2166136261u;

  for (u32_t i = 0; i < size_; ++i) {
    hash ^= data_[i];
    hash *= 16777619;
  }

  hash_ = hash;
}

int StringObject::find(StringObject* sub, u32_t off) const {
  if (sub->size_ == 0)
    return off;
  if (sub->size_ + off > size_)
    return -1;
  if (off >= size_)
    return -1;

  char* found = std::strstr(data_ + off, sub->data_);
  return found != nullptr ? Xt::as_type<int>(found - data_) : -1;
}

bool StringObject::is_equal(BaseObject* r) const {
  return false;
}

str_t StringObject::stringify() const {
  return "";
}

u32_t StringObject::hasher() const {
  return hash_;
}

}
