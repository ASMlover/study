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
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include "vm.hh"
#include "object.hh"

namespace nyx {

inline int power_of_2ceil(int n) {
  --n;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  ++n;

  return n;
}

static u32_t string_hash(const char* s, int n) {
  // FNV-1a hash. See: http://www.isthe.com/chongo/tech/comp/fnv/
  u32_t hash{2166136261u};
  for (int i = 0; i < n; ++i) {
    hash ^= s[i];
    hash *= 16777619;
  }
  return hash;
}

str_t BaseObject::type_name(void) const {
  switch (type_) {
  case ObjType::STRING: return "<string>";
  case ObjType::CLOSURE: return "<closure>";
  case ObjType::FUNCTION: return "<function>";
  case ObjType::NATIVE: return "<native>";
  case ObjType::UPVALUE: return "<upvalue>";
  case ObjType::CLASS: return "<class>";
  case ObjType::INSTANCE: return "<instance>";
  case ObjType::BOUND_METHOD: return "<bound method>";
  }
  return "<unknown>";
}

std::ostream& operator<<(std::ostream& out, BaseObject* obj) {
  return out << obj->stringify();
}

void gray_table(VM& vm, table_t& tbl) {
  for (auto& t : tbl)
    vm.gray_value(t.second);
}

void remove_table_undark(table_t& tbl) {
  for (auto it = tbl.begin(); it != tbl.end();) {
    if (!it->second.as_object()->is_dark())
      tbl.erase(it++);
    else
      ++it;
  }
}

StringObject::StringObject(const char* s, int n, u32_t hash, bool copy)
  : BaseObject(ObjType::STRING)
  , count_(n)
  , hash_(hash) {
  if (copy) {
    chars_ = new char[count_ + 1];
    if (s != nullptr)
      memcpy(chars_, s, n);
    chars_[count_] = 0;
  }
  else {
    chars_ = const_cast<char*>(s);
  }
}

StringObject::~StringObject(void) {
  if (chars_ != nullptr)
    delete [] chars_;
}

sz_t StringObject::size_bytes(void) const {
  return sizeof(*this) + sizeof(char) * count_;
}

str_t StringObject::stringify(void) const {
  return chars_;
}

bool StringObject::is_equal(BaseObject* other) const {
  return false;
  // auto* r = Xptr::down<StringObject>(other);
  // return (hash_ == r->hash_ &&  count_ == r->count_
  //     && memcmp(chars_, r->chars_, count_) == 0);
}

void StringObject::blacken(VM&) {
}

StringObject* StringObject::create(VM& vm, const str_t& s) {
  return create(vm, s.c_str(), static_cast<int>(s.size()));
}

StringObject* StringObject::create(VM& vm, const char* s, int n) {
  auto hash = string_hash(s, n);
  if (auto v = vm.get_intern_string(hash); v)
    return *v;

  auto* o = new StringObject(s, n, hash);
  vm.set_intern_string(hash, o);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::concat(VM& vm, StringObject* a, StringObject* b) {
  int count = 0;
  count += a == nullptr ? 0 : a->count();
  count += b == nullptr ? 0 : b->count();
  char* chars = new char[count + 1];

  int offset = 0;
  if (a != nullptr) {
    memcpy(chars, a->chars(), a->count());
    offset = a->count();
  }
  if (b != nullptr)
    memcpy(chars + offset, b->chars(), b->count());
  chars[count] = 0;
  u32_t hash = string_hash(chars, count);

  if (auto v = vm.get_intern_string(hash); v) {
    delete [] chars;
    return *v;
  }

  auto* o = new StringObject(chars, count, hash, true);
  vm.set_intern_string(hash, o);
  vm.append_object(o);
  return o;
}

FunctionObject::FunctionObject(void)
  : BaseObject(ObjType::FUNCTION) {
}

FunctionObject::~FunctionObject(void) {
}

sz_t FunctionObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t FunctionObject::stringify(void) const {
  std::stringstream ss;
  ss << "<fn `" << (name_ != nullptr ? name_->chars() : "<top>") << "`>";
  return ss.str();
}

bool FunctionObject::is_equal(BaseObject*) const {
  return false;
}

void FunctionObject::blacken(VM& vm) {
  chunk_.iter_constants([&vm](const Value& v) { vm.gray_value(v); });
  vm.gray_object(name_);
}

FunctionObject* FunctionObject::create(VM& vm) {
  auto* o = new FunctionObject();
  vm.append_object(o);
  return o;
}

sz_t NativeObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t NativeObject::stringify(void) const {
  std::stringstream ss;
  ss << "<native `" << &fn_ << "`>";
  return ss.str();
}

bool NativeObject::is_equal(BaseObject* other) const {
  return false;
}

void NativeObject::blacken(VM& vm) {
}

NativeObject* NativeObject::create(VM& vm, const NativeFunction& fn) {
  auto* o = new NativeObject(fn);
  vm.append_object(o);
  return o;
}

NativeObject* NativeObject::create(VM& vm, NativeFunction&& fn) {
  auto* o = new NativeObject(std::move(fn));
  vm.append_object(o);
  return o;
}

sz_t UpvalueObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t UpvalueObject::stringify(void) const {
  return "<upvalue>";
}

bool UpvalueObject::is_equal(BaseObject* other) const {
  return false;
}

void UpvalueObject::blacken(VM& vm) {
  vm.gray_value(closed_);
}

UpvalueObject* UpvalueObject::create(VM& vm, Value* slot) {
  auto* o = new UpvalueObject(slot);
  vm.append_object(o);
  return o;
}

ClosureObject::ClosureObject(FunctionObject* fn)
  : BaseObject(ObjType::CLOSURE)
  , function_(fn)
  , upvalues_count_(fn->upvalues_count()) {
  using UpvalueX = UpvalueObject*;

  upvalues_ = new UpvalueX[fn->upvalues_count()];
  for (int i = 0; i < fn->upvalues_count(); ++i)
    upvalues_[i] = nullptr;
}

ClosureObject::~ClosureObject(void) {
  if (upvalues_ != nullptr)
    delete [] upvalues_;
}

sz_t ClosureObject::size_bytes(void) const {
  return sizeof(*this) + sizeof(UpvalueObject*) * function_->upvalues_count();
}

str_t ClosureObject::stringify(void) const {
  std::stringstream ss;
  ss << "<fn `"
    << (function_->name() == nullptr ? "<top>" : function_->name()->chars())
    << "`>";
  return ss.str();
}

bool ClosureObject::is_equal(BaseObject* other) const {
  return false;
}

void ClosureObject::blacken(VM& vm) {
  vm.gray_object(function_);
  for (int i = 0; i < upvalues_count_; ++i)
    vm.gray_object(upvalues_[i]);
}

ClosureObject* ClosureObject::create(VM& vm, FunctionObject* fn) {
  auto* o = new ClosureObject(fn);
  vm.append_object(o);
  return o;
}

ClassObject::ClassObject(
    StringObject* name, ClassObject* superclass)
  : BaseObject(ObjType::CLASS)
  , name_(name)
  , superclass_(superclass) {
}

ClassObject::~ClassObject(void) {
}

void ClassObject::inherit_from(ClassObject* superclass) {
  for (auto& meth : superclass->methods_)
    methods_[meth.first] = meth.second;
}

sz_t ClassObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t ClassObject::stringify(void) const {
  std::stringstream ss;
  ss << "<`" << name_->chars() << "` class>";
  return ss.str();
}

bool ClassObject::is_equal(BaseObject* other) const {
  return false;
}

void ClassObject::blacken(VM& vm) {
  vm.gray_object(name_);
  vm.gray_object(superclass_);
  gray_table(vm, methods_);
}

ClassObject* ClassObject::create(
    VM& vm, StringObject* name, ClassObject* superclass) {
  auto* o = new ClassObject(name, superclass);
  vm.append_object(o);
  return o;
}

InstanceObject::InstanceObject(ClassObject* klass)
  : BaseObject(ObjType::INSTANCE) , class_(klass) {
}

InstanceObject::~InstanceObject(void) {
}

sz_t InstanceObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t InstanceObject::stringify(void) const {
  std::stringstream ss;
  ss << "<`" << class_->name()->chars() << "` instance>";
  return ss.str();
}

bool InstanceObject::is_equal(BaseObject* other) const {
  return false;
}

void InstanceObject::blacken(VM& vm) {
  vm.gray_object(class_);
  gray_table(vm, fields_);
}

InstanceObject* InstanceObject::create(VM& vm, ClassObject* klass) {
  auto* o = new InstanceObject(klass);
  vm.append_object(o);
  return o;
}

BoundMethodObject::BoundMethodObject(const Value& receiver, ClosureObject* method)
  : BaseObject(ObjType::BOUND_METHOD)
  , receiver_(receiver)
  , method_(method) {
}

BoundMethodObject::~BoundMethodObject(void) {
}

sz_t BoundMethodObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t BoundMethodObject::stringify(void) const {
  std::stringstream ss;
  ss << "<fn `" << method_->get_function()->name()->chars() << "`";
  return ss.str();
}

bool BoundMethodObject::is_equal(BaseObject* other) const {
  return false;
}

void BoundMethodObject::blacken(VM& vm) {
  vm.gray_value(receiver_);
  vm.gray_object(method_);
}

BoundMethodObject* BoundMethodObject::create(
    VM& vm, const Value& receiver, ClosureObject* method) {
  auto* o = new BoundMethodObject(receiver, method);
  vm.append_object(o);
  return o;
}

}
