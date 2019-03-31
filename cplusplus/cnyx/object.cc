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
#include <iostream>
#include <sstream>
#include "object.hh"
#include "vm.hh"

namespace nyx {

std::ostream& operator<<(std::ostream& out, Object* o) {
  return out << o->stringify();
}

template <typename T> inline T* __offset_of(void* startp, std::size_t offset) {
  return reinterpret_cast<T*>(reinterpret_cast<byte_t*>(startp) + offset);
}

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

ArrayObject::ArrayObject(int capacity)
  : Object(ObjType::ARRAY)
  , capacity_(capacity) {
  elements_ = __offset_of<Value>(this, sizeof(*this));
  for (int i = 0; i < capacity_; ++i)
    elements_[i] = nullptr;
}

std::size_t ArrayObject::size(void) const {
  return sizeof(*this) + sizeof(Value) * capacity_;
}

std::string ArrayObject::stringify(void) const {
  std::stringstream ss;
  ss << "array(" << count_ << ")";
  return ss.str();
}

void ArrayObject::traverse(VM& vm) {
  for (int i = 0; i < count_; ++i)
    elements_[i] = vm.move_object(elements_[i]);
}

Object* ArrayObject::move_to(void* p) {
  return new (p) ArrayObject(std::move(*this));
}

ArrayObject* ArrayObject::create(VM& vm, int capacity) {
  void* p = vm.allocate(sizeof(ArrayObject) + capacity * sizeof(Value));
  return new (p) ArrayObject(capacity);
}

ArrayObject* ArrayObject::ensure(
    VM& vm, ArrayObject* orig_array, int new_capacity) {
  int orig_capacity = orig_array == nullptr ? 0 : orig_array->capacity();
  if (orig_capacity >= new_capacity)
    return orig_array;

  new_capacity = power_of_2ceil(new_capacity);
  auto* new_array = ArrayObject::create(vm, new_capacity);
  if (orig_array != nullptr) {
    for (int i = 0; i < orig_array->count(); ++i)
      new_array->append_element(orig_array->get_element(i));
  }
  return new_array;
}

std::size_t ForwardObject::size(void) const {
  return sizeof(*this);
}

std::string ForwardObject::stringify(void) const {
  std::stringstream ss;
  ss << "fwd->" << to_->address();
  return ss.str();
}

void ForwardObject::traverse(VM& vm) {
  assert(false);
}

Object* ForwardObject::move_to(void* p) {
  return new (p) ForwardObject(std::move(*this));
}

ForwardObject* ForwardObject::forward(void* p) {
  return new (p) ForwardObject();
}

ForwardObject* ForwardObject::create(VM& vm) {
  return new (vm.allocate(sizeof(ForwardObject))) ForwardObject();
}

std::size_t NumericObject::size(void) const {
  return sizeof(*this);
}

std::string NumericObject::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

void NumericObject::traverse(VM&) {
}

Object* NumericObject::move_to(void* p) {
  return new (p) NumericObject(std::move(*this));
}

NumericObject* NumericObject::create(VM& vm, double d) {
  return new (vm.allocate(sizeof(NumericObject))) NumericObject(d);
}

StringObject::StringObject(int capacity)
  : Object(ObjType::STRING)
  , capacity_(capacity) {
  chars_ = __offset_of<std::uint8_t>(this, sizeof(*this));
  chars_[count_] = 0;
}

void StringObject::set_chars(const std::uint8_t* s, int n) {
  count_ = n;
  memcpy(chars_, s, n);
  chars_[count_] = 0;
}

std::size_t StringObject::size(void) const {
  return sizeof(*this) + sizeof(std::uint8_t) * capacity_;
}

std::string StringObject::stringify(void) const {
  std::stringstream ss;
  ss << "string(" << count_ << ")";
  return ss.str();
}

void StringObject::traverse(VM&) {
}

Object* StringObject::move_to(void* p) {
  return new (p) StringObject(std::move(*this));
}

StringObject* StringObject::create(VM& vm, int capacity) {
  void* p = vm.allocate(
      sizeof(StringObject) + (capacity + 1) * sizeof(std::uint8_t));
  return new (p) StringObject(capacity);
}

StringObject* StringObject::create(VM& vm, const std::uint8_t* s, int n) {
  int capacity = power_of_2ceil(n);
  StringObject* str = create(vm, capacity);
  str->set_chars(s, n);
  return str;
}

StringObject* StringObject::ensure(
    VM& vm, StringObject* orig_str, int new_capacity) {
  int orig_capacity = orig_str == nullptr ? 0 : orig_str->capacity();
  if (orig_capacity >= new_capacity)
    return orig_str;

  new_capacity = power_of_2ceil(new_capacity);
  auto* new_str = StringObject::create(vm, new_capacity);
  if (orig_str != nullptr)
    new_str->set_chars(orig_str->chars(), orig_str->count());
  return new_str;
}

FunctionObject::FunctionObject(void)
  : Object(ObjType::FUNCTION) {
}

void FunctionObject::dump(void) {
  const auto* codes = raw_codes();
  int n = codes_count();
  for (int i = 0; i < n;) {
    switch (codes[i++]) {
    case OpCode::OP_CONSTANT:
      {
        std::uint8_t constant = codes[i++];
        fprintf(stdout, "%-10s %5d `", "OP_CONSTANT", constant);
        std::cout << constants_->get_element(constant) << "`" << std::endl;
      } break;
    case OpCode::OP_ADD: std::cout << "OP_ADD" << std::endl; break;
    case OpCode::OP_SUB: std::cout << "OP_SUB" << std::endl; break;
    case OpCode::OP_MUL: std::cout << "OP_MUL" << std::endl; break;
    case OpCode::OP_DIV: std::cout << "OP_DIV" << std::endl; break;
    case OpCode::OP_RETURN: std::cout << "OP_RETURN" << std::endl; break;
    }
  }
}

std::size_t FunctionObject::size(void) const {
  return sizeof(*this);
}

std::string FunctionObject::stringify(void) const {
  // TODO:
  return "function";
}

void FunctionObject::traverse(VM& vm) {
  constants_ = vm.move_object(constants_)->down_to<ArrayObject>();
  codes_ = vm.move_object(codes_)->down_to<StringObject>();
}

Object* FunctionObject::move_to(void* p) {
  return new (p) FunctionObject(std::move(*this));
}

FunctionObject* FunctionObject::create(VM& vm) {
  return new (vm.allocate(sizeof(FunctionObject))) FunctionObject();
}

std::size_t TableEntriesObject::size(void) const {
  return sizeof(*this) + sizeof(TableEntry) * count_;
}

std::string TableEntriesObject::stringify(void) const {
  // TODO:
  return "table entries";
}

void TableEntriesObject::traverse(VM& vm) {
  for (int i = 0; i < count_; ++i) {
    auto& entry = entries_[i];
    entry.key = vm.move_object(entry.key);
    entry.value = vm.move_object(entry.value);
  }
}

Object* TableEntriesObject::move_to(void* p) {
  return new (p) TableEntriesObject(std::move(*this));
}

std::size_t TableObject::size(void) const {
  return sizeof(*this);
}

std::string TableObject::stringify(void) const {
  // TODO:
  return "table";
}

void TableObject::traverse(VM& vm) {
  entries_ = vm.move_object(entries_)->down_to<TableEntriesObject>();
}

Object* TableObject::move_to(void* p) {
  return new (p) TableObject(std::move(*this));
}

TableObject* TableObject::create(VM& vm) {
  return new (vm.allocate(sizeof(TableObject))) TableObject();
}

}
