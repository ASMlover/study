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

StringObject* Value::as_string(void) const {
  return Xt::down<StringObject>(obj_);
}

const char* Value::as_cstring(void) const {
  return Xt::down<StringObject>(obj_)->cstr();
}

FunctionObject* Value::as_function(void) const {
  return Xt::down<FunctionObject>(obj_);
}

ClassObject* Value::as_class(void) const {
  return Xt::down<ClassObject>(obj_);
}

InstanceObject* Value::as_instance(void) const {
  return Xt::down<InstanceObject>(obj_);
}

str_t Value::stringify(void) const {
  switch (type_) {
  case ValueType::NIL: return "nil";
  case ValueType::TRUE: return "true";
  case ValueType::FALSE: return "false";
  case ValueType::NUMERIC: return Xt::to_string(num_);
  case ValueType::OBJECT: return obj_->stringify();
  }
  return "";
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

StringObject* StringObject::make_string(VM& vm, const char* s, int n) {
  auto* o = new StringObject(s, n);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::make_string(VM& vm, const str_t& s) {
  return make_string(vm, s.data(), Xt::as_type<int>(s.size()));
}

StringObject* StringObject::make_string(
    VM& vm, StringObject* s1, StringObject* s2) {
  int n = s1->size() + s2->size();
  char* s = new char [Xt::as_type<sz_t>(n + 1)];
  memcpy(s, s1->cstr(), s1->size());
  memcpy(s + s1->size(), s2->cstr(), s2->size());
  s[n] = 0;

  auto* o = new StringObject(s, n, true);
  vm.append_object(o);
  return o;
}

str_t FunctionObject::stringify(void) const {
  std::stringstream ss;
  ss << "[fn `" << this << "`]";
  return ss.str();
}

void FunctionObject::gc_mark(VM& vm) {
  for (auto& c : constants_)
    vm.mark_value(c);
}

FunctionObject* FunctionObject::make_function(VM& vm) {
  auto* o = new FunctionObject();
  vm.append_object(o);
  return o;
}

ClassObject::ClassObject(void) noexcept
  : BaseObject(ObjType::CLASS) {
}

ClassObject::ClassObject(ClassObject* meta_class, ClassObject* supercls) noexcept
  : BaseObject(ObjType::CLASS)
  , meta_class_(meta_class)
  , superclass_(supercls) {
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

void ClassObject::gc_mark(VM& vm) {
  vm.mark_object(meta_class_);
  for (auto& m : methods_) {
    if (m.type == MethodType::BLOCK)
      vm.mark_object(m.fn);
  }
}

ClassObject* ClassObject::make_class(VM& vm, ClassObject* superclass) {
  auto* meta_class = new ClassObject(nullptr, nullptr);
  auto* o = new ClassObject(meta_class, superclass);
  vm.append_object(o);
  return o;
}

InstanceObject::InstanceObject(ClassObject* cls) noexcept
  : BaseObject(ObjType::INSTANCE)
  , cls_(cls) {
}

str_t InstanceObject::stringify(void) const {
  std::stringstream ss;
  ss << "[instance `" << this << "`]";
  return ss.str();
}

InstanceObject* InstanceObject::make_instance(VM& vm, ClassObject* cls) {
  auto* o = new InstanceObject(cls);
  vm.append_object(o);
  return o;
}

}
