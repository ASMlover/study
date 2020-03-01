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
  return Xt::down<StringObject>(this);
}

const char* BaseObject::as_cstring() noexcept {
  return Xt::down<StringObject>(this)->cstr();
}

ListObject* BaseObject::as_list() noexcept {
  return Xt::down<ListObject>(this);
}

RangeObject* BaseObject::as_range() noexcept {
  return Xt::down<RangeObject>(this);
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

StringObject::StringObject(ClassObject* cls, char c) noexcept
  : BaseObject(ObjType::STRING, cls)
  , size_(1) {
  data_ = new char[size_ + 1];
  data_[0] = c, data_[1] = 0;

  hash_string();
}

StringObject::StringObject(
    ClassObject* cls, const char* s, sz_t n, bool replace_owner) noexcept
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

void StringObject::hash_string() noexcept {
  // FNV-1a hash. See: http://www.isthe.com/chongo/tech/comp/fnv/

  u32_t hash = 2166136261u;
  for (u32_t i = 0; i < size_; ++i) {
    hash ^= data_[i];
    hash *= 16777619;
  }
  hash_ = hash;
}

int StringObject::find(StringObject* sub, sz_t off) const {
  if (sub->size_ == 0)
    return Xt::as_type<int>(off);
  if (sub->size_ + off > size_ || off >= size_)
    return -1;

  char* found = std::strstr(data_ + off, sub->data_);
  return found != nullptr ? Xt::as_type<int>(found - data_) : -1;
}

bool StringObject::is_equal(BaseObject* r) const {
  return compare(r->as_string());
}

str_t StringObject::stringify() const {
  return data_ != nullptr ? data_ : "";
}

u32_t StringObject::hasher() const {
  return hash_;
}

StringObject* StringObject::create(WrenVM& vm, char c) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::create(WrenVM& vm, const char* s, sz_t n) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::create(WrenVM& vm, const str_t& s) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::concat(
    WrenVM& vm, StringObject* s1, StringObject* s2) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::concat(WrenVM& vm, const char* s1, const char* s2) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::concat(
    WrenVM& vm, const str_t& s1, const str_t& s2) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::from_byte(WrenVM& vm, u8_t value) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::from_numeric(WrenVM& vm, double value) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::from_range(
    WrenVM& vm, StringObject* s, sz_t off, sz_t n, sz_t step) {
  // TODO:
  return nullptr;
}

StringObject* StringObject::format(WrenVM& vm, const char* format, ...) {
  // TODO:
  return nullptr;
}

ListObject::ListObject(ClassObject* cls, sz_t n, const Value& v) noexcept
  : BaseObject(ObjType::LIST, cls) {
  if (n > 0)
    elements_.resize(n, v);
}

str_t ListObject::stringify() const {
  ss_t ss;

  ss << "[";
  for (auto i = 0u; i < elements_.size(); ++i) {
    if (i > 0)
      ss << ", ";

    auto& v = elements_[i];
    if (v.is_string())
      ss << "\"" << v << "\"";
    else
      ss << v;
  }
  ss << "]";

  return ss.str();
}

void ListObject::gc_blacken(WrenVM& vm) {
  // TODO:
}

ListObject* ListObject::create(WrenVM& vm, sz_t n, const Value& v) {
  // TODO:
  return nullptr;
}

bool RangeObject::is_equal(BaseObject* r) const {
  RangeObject* o = r->as_range();
  return from_ == o->from_ && to_ == o->to_ && is_inclusive_ == o->is_inclusive_;
}

str_t RangeObject::stringify() const {
  ss_t ss;
  ss << "[range `" << this << "`]";
  return ss.str();
}

u32_t RangeObject::hasher() const {
  return Xt::hash_numeric(from_) ^ Xt::hash_numeric(to_);
}

RangeObject* RangeObject::create(
    WrenVM& vm, double from, double to, bool is_inclusive) {
  // TODO:
  return nullptr;
}

}
