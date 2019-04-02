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

std::size_t BooleanObject::size(void) const {
  return sizeof(*this);
}

std::string BooleanObject::stringify(void) const {
  return value_ ? "true" : "false";
}

void BooleanObject::blacken(VM& vm) {
}

BooleanObject* BooleanObject::create(VM& vm, bool b) {
  auto* o = new BooleanObject(b);
  vm.put_in(o);
  return o;
}

std::size_t NumericObject::size(void) const {
  return sizeof(*this);
}

std::string NumericObject::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

void NumericObject::blacken(VM&) {
}

NumericObject* NumericObject::create(VM& vm, double d) {
  auto* o = new NumericObject(d);
  vm.put_in(o);
  return o;
}

StringObject::StringObject(const char* s, int n)
  : Object(ObjType::STRING)
  , count_(n) {
  chars_ = new char[count_ + 1];
  if (s != nullptr)
    memcpy(chars_, s, n);
  chars_[count_] = 0;
}

StringObject::~StringObject(void) {
  if (chars_ != nullptr)
    delete [] chars_;
}

void StringObject::inti_from_string(const char* s, int n) {
  if (chars_ != s || count_ != n) {
    count_ = n;
    chars_ = const_cast<char*>(s);
  }
}

std::size_t StringObject::size(void) const {
  return sizeof(*this) + sizeof(char) * count_;
}

std::string StringObject::stringify(void) const {
  return chars_;
}

void StringObject::blacken(VM&) {
}

StringObject* StringObject::create(VM& vm, const char* s, int n) {
  auto* o = new StringObject(s, n);
  vm.put_in(o);
  return o;
}

StringObject* StringObject::concat(VM& vm, StringObject* a, StringObject* b) {
  int n = a->count() + b->count();
  char* s = new char[n + 1];
  memcpy(s, a->chars(), a->count());
  memcpy(s + a->count(), b->chars(), b->count());
  s[n] = 0;

  auto* o = new StringObject();
  o->inti_from_string(s, n);
  vm.put_in(o);
  return o;
}

FunctionObject::FunctionObject(void)
  : Object(ObjType::FUNCTION) {
}

FunctionObject::~FunctionObject(void) {
  if (codes_ != nullptr)
    delete [] codes_;
  if (constants_ != nullptr)
    delete [] constants_;
}

void FunctionObject::dump(void) {
  const auto* codes = codes_;
  for (int i = 0; i < codes_count_;) {
    switch (codes[i++]) {
    case OpCode::OP_CONSTANT:
      {
        std::uint8_t constant = codes[i++];
        fprintf(stdout, "%-10s %5d `", "OP_CONSTANT", constant);
        std::cout << constants_[constant] << "`" << std::endl;
      } break;
    case OpCode::OP_ADD: std::cout << "OP_ADD" << std::endl; break;
    case OpCode::OP_SUB: std::cout << "OP_SUB" << std::endl; break;
    case OpCode::OP_MUL: std::cout << "OP_MUL" << std::endl; break;
    case OpCode::OP_DIV: std::cout << "OP_DIV" << std::endl; break;
    case OpCode::OP_RETURN: std::cout << "OP_RETURN" << std::endl; break;
    }
  }
}

void FunctionObject::append_code(std::uint8_t c) {
  if (codes_capacity_ < codes_count_ + 1) {
    codes_capacity_ = codes_capacity_ == 0 ? 4 : codes_capacity_ * 2;

    auto* new_codes = new std::uint8_t[codes_capacity_];
    memcpy(new_codes, codes_, sizeof(std::uint8_t) * codes_count_);
    codes_ = new_codes;
  }
  codes_[codes_count_++] = c;
}

void FunctionObject::append_constant(Value v) {
  if (constants_capacity_ < constants_count_ + 1) {
    constants_capacity_ = constants_capacity_ == 0 ? 4 : constants_capacity_ * 2;

    auto* new_constants = new Value[constants_capacity_];
    memcpy(new_constants, constants_, sizeof(Value) * constants_count_);
    constants_ = new_constants;
  }
  constants_[constants_count_++] = v;
}

std::size_t FunctionObject::size(void) const {
  return sizeof(*this);
}

std::string FunctionObject::stringify(void) const {
  // TODO:
  return "function";
}

void FunctionObject::blacken(VM& vm) {
  for (int i = 0; i < constants_count_; ++i)
    vm.gray_value(constants_[i]);
}

FunctionObject* FunctionObject::create(VM& vm) {
  auto* o = new FunctionObject();
  vm.put_in(o);
  return o;
}

TableObject::~TableObject(void) {
  if (entries_ != nullptr)
    delete [] entries_;
}

std::size_t TableObject::size(void) const {
  return sizeof(*this);
}

std::string TableObject::stringify(void) const {
  // TODO:
  return "table";
}

void TableObject::blacken(VM& vm) {
  for (int i = 0; i < capacity_; ++i) {
    auto& entry = entries_[i];
    vm.gray_value(entry.key);
    vm.gray_value(entry.value);
  }
}

TableObject* TableObject::create(VM& vm) {
  auto* o = new TableObject();
  vm.put_in(o);
  return o;
}

}
