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
#include "bytecc_value.hh"

namespace loxcc::bytecc {

template <typename T, typename... Args>
inline T* make_object(VM& vm, Args&&... args) {
  auto* o = new T(std::forward<Args>(args)...);
  // TODO: append object to VM
  return o;
}

StringObject* Value::as_string(void) const {
  return Xt::down<StringObject>(as_.object);
}

const char* Value::as_cstring(void) const {
  return Xt::down<StringObject>(as_.object)->cstr();
}

Value::NativeTp Value::as_native(void) const {
  return Xt::down<NativeObject>(as_.object)->fn();
}

FunctionObject* Value::as_function(void) const {
  return Xt::down<FunctionObject>(as_.object);
}

UpvalueObject* Value::as_upvalue(void) const {
  return Xt::down<UpvalueObject>(as_.object);
}

ClosureObject* Value::as_closure(void) const {
  return Xt::down<ClosureObject>(as_.object);
}

ClassObject* Value::as_class(void) const {
  return Xt::down<ClassObject>(as_.object);
}

InstanceObject* Value::as_instance(void) const {
  return Xt::down<InstanceObject>(as_.object);
}

BoundMehtodObject* Value::as_bound_method(void) const {
  return Xt::down<BoundMehtodObject>(as_.object);
}

bool Value::operator==(const Value& r) const {
  if (this == &r)
    return true;
  if (type_ != r.type_)
    return false;

  switch (type_) {
  case ValueType::NIL: return true;
  case ValueType::BOOLEAN: return as_.boolean == r.as_.boolean;
  case ValueType::NUMERIC: return as_.numeric == r.as_.numeric;
  case ValueType::OBJECT: return as_.object == r.as_.object; // TODO: FIXME:
  }
  return false;
}

bool Value::operator!=(const Value& r) const {
  return !(*this == r);
}

bool Value::is_truthy(void) const {
  switch (type_) {
  case ValueType::NIL: return false;
  case ValueType::BOOLEAN: return as_.boolean;
  case ValueType::NUMERIC: return as_.numeric == 0.f;
  case ValueType::OBJECT: return as_.object->is_truthy();
  }
  return false;
}

str_t Value::stringify(void) const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::BOOLEAN: return as_.boolean ? "true" : "false";
  case ValueType::NUMERIC: return Xt::to_string(as_.numeric);
  case ValueType::OBJECT: return as_.object->stringify();
  }
  return "";
}

StringObject::StringObject(
    const char* s, int n, u32_t h, bool replace_owner) noexcept
  : BaseObject(ObjType::STRING)
  , size_(n)
  , hash_(h) {
  if (replace_owner) {
    data_ = Xt::as_ptr<char>(s);
  }
  else {
    data_ = new char[size_ + 1];
    memcpy(data_, s, size_);
    data_[size_] = 0;
  }
}

StringObject::~StringObject(void) {
  delete [] data_;
}

sz_t StringObject::size_bytes(void) const {
  return sizeof(*this) + sizeof(char) * size_;
}

str_t StringObject::stringify(void) const {
  return data_;
}

StringObject* StringObject::create(VM& vm, const str_t& s) {
  return create(vm, s.c_str(), Xt::as_type<int>(s.size()));
}

StringObject* StringObject::create(VM& vm, const char* s, int n) {
  u32_t h = Xt::hasher(s, n);
  // TODO: set interned strings
  return make_object<StringObject>(vm, s, n, h);
}

StringObject* StringObject::concat(VM& vm, StringObject* a, StringObject* b) {
  int n = a->size() + b->size();
  char* s = new char[n + 1];
  memcpy(s, a->data(), a->size());
  memcpy(s + a->size(), b->data(), b->size());
  s[n] = 0;

  u32_t h = Xt::hasher(s, n);
  // TODO: set interned strings
  return make_object<StringObject>(vm, s, n, h, true);
}

}
