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
#include <sstream>
#include "object.hh"
#include "vm.hh"

namespace nyx {

std::ostream& operator<<(std::ostream& out, Object* o) {
  return out << o->stringify();
}

template <typename T> inline T* __offset_of(void* startptr, std::size_t offset) {
  return reinterpret_cast<T*>(reinterpret_cast<byte_t*>(startptr) + offset);
}

Array::Array(int count)
  : Object(ObjType::ARRAY)
  , count_(count) {
  elements_ = __offset_of<Value>(this, sizeof(*this));
  for (int i = 0; i < count_; ++i)
    elements_[i] = nullptr;
}

std::size_t Array::size(void) const {
  return sizeof(*this) + count_ * sizeof(Value);
}

std::string Array::stringify(void) const {
  // TODO:
  return "array";
}

void Array::traverse(VM* vm) {
  for (int i = 0; i < count_; ++i)
    elements_[i] = vm->move_object(elements_[i]);
}

Object* Array::move_to(void* p) {
  return new (p) Array(std::move(*this));
}

Array* Array::create(VM* vm, int count) {
  void* p = vm->allocate(sizeof(Array) + count * sizeof(Value));
  return new (p) Array(count);
}

std::size_t Forward::size(void) const {
  return sizeof(*this);
}

std::string Forward::stringify(void) const {
  std::stringstream ss;
  ss << "fwd->" << to_->address();
  return ss.str();
}

void Forward::traverse(VM* vm) {
  assert(false);
}

Object* Forward::move_to(void* p) {
  return new (p) Forward(std::move(*this));
}

Forward* Forward::create(VM* vm) {
  return new (vm->allocate(sizeof(Forward))) Forward();
}

Function::Function(Array* constants, std::uint8_t* codes, int code_size)
  : Object(ObjType::FUNCTION)
  , constants_(constants)
  , code_size_(code_size) {
  codes_ = __offset_of<std::uint8_t>(this, sizeof(*this));
  memcpy(codes_, codes, sizeof(std::uint8_t) * code_size_);
}

std::size_t Function::size(void) const {
  return sizeof(*this) + sizeof(std::uint8_t) * code_size_;
}

std::string Function::stringify(void) const {
  // TODO:
  return "function";
}

void Function::traverse(VM* vm) {
  constants_ = vm->move_object(constants_)->down_to<Array>();
}

Object* Function::move_to(void* p) {
  return new (p) Function(std::move(*this));
}

Function* Function::create(VM* vm,
    Array* constants, std::uint8_t* codes, int code_size) {
  void* p = vm->allocate(sizeof(Function) + sizeof(std::uint8_t) * code_size);
  return new (p) Function(constants, codes, code_size);
}

std::size_t Numeric::size(void) const {
  return sizeof(*this);
}

std::string Numeric::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

void Numeric::traverse(VM*) {
}

Object* Numeric::move_to(void* p) {
  return new (p) Numeric(std::move(*this));
}

Numeric* Numeric::create(VM* vm, double d) {
  return new (vm->allocate(sizeof(Numeric))) Numeric(d);
}

String::String(const char* s, int n)
  : Object(ObjType::STRING)
  , count_(n) {
  chars_ = __offset_of<char>(this, sizeof(*this));
  std::memcpy(chars_, s, n);
  chars_[count_] = 0;
}

std::size_t String::size(void) const {
  return sizeof(*this) + count_;
}

std::string String::stringify(void) const {
  return chars_;
}

void String::traverse(VM*) {
}

Object* String::move_to(void* p) {
  return new (p) String(std::move(*this));
}

String* String::create(VM* vm, const char* s, int n) {
  void* p = vm->allocate(sizeof(String) + (n + 1) * sizeof(char));
  return new (p) String(s, n);
}

std::size_t TableEntries::size(void) const {
  return sizeof(*this) + count_ * sizeof(TableEntry);
}

std::string TableEntries::stringify(void) const {
  // TODO:
  return "table entries";
}

void TableEntries::traverse(VM* vm) {
  for (int i = 0; i < count_; ++i) {
    auto& entry = entries_[i];
    entry.key = vm->move_object(entry.key);
    entry.value = vm->move_object(entry.value);
  }
}

Object* TableEntries::move_to(void* p) {
  return new (p) TableEntries(std::move(*this));
}

std::size_t Table::size(void) const {
  return sizeof(*this);
}

std::string Table::stringify(void) const {
  // TODO:
  return "table";
}

void Table::traverse(VM* vm) {
  entries_ = vm->move_object(entries_)->down_to<TableEntries>();
}

Object* Table::move_to(void* p) {
  return new (p) Table(std::move(*this));
}

Table* Table::create(VM* vm) {
  return new (vm->allocate(sizeof(Table))) Table();
}

}
