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
#include <sstream>
#include "vm.hh"
#include "value.hh"

namespace wrencc {

std::ostream& operator<<(std::ostream& out, const Value& val) {
  return out << val.stringify();
}

StringObject* TagValue::as_string(void) const {
  return Xt::down<StringObject>(as_object());
}

const char* TagValue::as_cstring(void) const {
  return Xt::down<StringObject>(as_object())->cstr();
}

ListObject* TagValue::as_list(void) const {
  return Xt::down<ListObject>(as_object());
}

FunctionObject* TagValue::as_function(void) const {
  return Xt::down<FunctionObject>(as_object());
}

ClassObject* TagValue::as_class(void) const {
  return Xt::down<ClassObject>(as_object());
}

InstanceObject* TagValue::as_instance(void) const {
  return Xt::down<InstanceObject>(as_object());
}

str_t TagValue::stringify(void) const {
  if (is_numeric()) {
    return Xt::to_string(as_numeric());
  }
  else if (is_object()) {
    return as_object()->stringify();
  }
  else {
    switch (tag()) {
    case Tag::NaN: return "NaN";
    case Tag::NIL: return "nil";
    case Tag::TRUE: return "true";
    case Tag::FALSE: return "false";
    }
  }
  return "";
}

StringObject* ObjValue::as_string(void) const {
  return Xt::down<StringObject>(obj_);
}

const char* ObjValue::as_cstring(void) const {
  return Xt::down<StringObject>(obj_)->cstr();
}

ListObject* ObjValue::as_list(void) const {
  return Xt::down<ListObject>(obj_);
}

FunctionObject* ObjValue::as_function(void) const {
  return Xt::down<FunctionObject>(obj_);
}

ClassObject* ObjValue::as_class(void) const {
  return Xt::down<ClassObject>(obj_);
}

InstanceObject* ObjValue::as_instance(void) const {
  return Xt::down<InstanceObject>(obj_);
}

str_t ObjValue::stringify(void) const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::TRUE: return "true";
  case ValueType::FALSE: return "false";
  case ValueType::NUMERIC: return Xt::to_string(num_);
  case ValueType::OBJECT: return obj_->stringify();
  }
  return "";
}

StringObject::StringObject(char c) noexcept
  : BaseObject(ObjType::STRING)
  , size_(1) {
  value_ = new char[2];
  value_[0] = c;
  value_[1] = 0;
}

StringObject::StringObject(const char* s, int n, bool replace_owner) noexcept
  : BaseObject(ObjType::STRING)
  , size_(n) {
  if (replace_owner) {
    value_ = const_cast<char*>(s);
  }
  else {
    value_ = new char[Xt::as_type<sz_t>(size_ + 1)];
    if (s != nullptr) {
      memcpy(value_, s, n);
      value_[size_] = 0;
    }
  }
}

StringObject::~StringObject(void) {
  delete [] value_;
}

str_t StringObject::stringify(void) const {
  return value_;
}

StringObject* StringObject::make_string(WrenVM& vm, char c) {
  auto* o = new StringObject(c);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::make_string(WrenVM& vm, const char* s, int n) {
  auto* o = new StringObject(s, n);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::make_string(WrenVM& vm, const str_t& s) {
  return make_string(vm, s.data(), Xt::as_type<int>(s.size()));
}

StringObject* StringObject::make_string(
    WrenVM& vm, StringObject* s1, StringObject* s2) {
  int n = s1->size() + s2->size();
  char* s = new char [Xt::as_type<sz_t>(n + 1)];
  memcpy(s, s1->cstr(), s1->size());
  memcpy(s + s1->size(), s2->cstr(), s2->size());
  s[n] = 0;

  auto* o = new StringObject(s, n, true);
  vm.append_object(o);
  return o;
}

ListObject::ListObject(int num_elements) noexcept
  : BaseObject(ObjType::LIST) {
  if (num_elements > 0)
    elements_.resize(num_elements);
}

str_t ListObject::stringify(void) const {
  std::stringstream ss;
  ss << "[list: " << this << "]";
  return ss.str();
}

void ListObject::gc_mark(WrenVM& vm) {
  for (auto& e : elements_)
    vm.mark_value(e);
}

ListObject* ListObject::make_list(WrenVM& vm, int num_elements) {
  auto* o = new ListObject(num_elements);
  vm.append_object(o);
  return o;
}

str_t FunctionObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fn `" << this << "`]";
  return ss.str();
}

void FunctionObject::gc_mark(WrenVM& vm) {
  for (auto& c : constants_)
    vm.mark_value(c);
}

FunctionObject* FunctionObject::make_function(WrenVM& vm) {
  auto* o = new FunctionObject();
  vm.append_object(o);
  return o;
}

ClassObject::ClassObject(void) noexcept
  : BaseObject(ObjType::CLASS) {
}

ClassObject::ClassObject(
    ClassObject* meta_class, ClassObject* supercls, int num_fields) noexcept
  : BaseObject(ObjType::CLASS)
  , meta_class_(meta_class)
  , superclass_(supercls)
  , num_fields_(num_fields) {
  if (superclass_ != nullptr) {
    for (int i = 0; i < kMaxMethods; ++i)
      methods_[i] = superclass_->methods_[i];
  }
}

str_t ClassObject::stringify(void) const {
  std::stringstream ss;
  ss << "[class `" << this << "`]";
  return ss.str();
}

void ClassObject::gc_mark(WrenVM& vm) {
  vm.mark_object(meta_class_);
  for (auto& m : methods_) {
    if (m.type == MethodType::BLOCK)
      vm.mark_object(m.fn);
  }
}

ClassObject* ClassObject::make_class(
    WrenVM& vm, ClassObject* superclass, int num_fields) {
  auto* meta_class = new ClassObject(nullptr, nullptr, 0);
  auto* o = new ClassObject(meta_class, superclass, num_fields);
  vm.append_object(o);
  return o;
}

InstanceObject::InstanceObject(ClassObject* cls) noexcept
  : BaseObject(ObjType::INSTANCE)
  , cls_(cls)
  , fields_(cls->num_fields()) {
  for (int i = 0; i < cls_->num_fields(); ++i)
    fields_[i] = nullptr;
}

str_t InstanceObject::stringify(void) const {
  std::stringstream ss;
  ss << "[instance `" << this << "`]";
  return ss.str();
}

void InstanceObject::gc_mark(WrenVM& vm) {
  for (auto& v : fields_)
    vm.mark_value(v);
}

InstanceObject* InstanceObject::make_instance(WrenVM& vm, ClassObject* cls) {
  auto* o = new InstanceObject(cls);
  vm.append_object(o);
  return o;
}

}
