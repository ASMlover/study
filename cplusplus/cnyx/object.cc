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

template <typename T> inline T* __offset_of(void* startp, sz_t offset) {
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

ValueArray::~ValueArray(void) {
  if (values_ != nullptr)
    delete [] values_;
}

int ValueArray::append_value(Value v) {
  if (capacity_ < count_ + 1) {
    capacity_ = capacity_ == 0 ? 4 : capacity_ * 2;

    auto* new_values = new Value[capacity_];
    if (values_ != nullptr) {
      memcpy(new_values, values_, sizeof(Value) * count_);
      delete [] values_;
    }
    values_ = new_values;
  }

  values_[count_] = v;
  return count_++;
}

void ValueArray::gray(VM& vm) {
  for (int i = 0; i < count_; ++i)
    vm.gray_value(values_[i]);
}

sz_t BooleanObject::size(void) const {
  return sizeof(*this);
}

str_t BooleanObject::stringify(void) const {
  return value_ ? "true" : "false";
}

bool BooleanObject::is_equal(Object* other) const {
  return value_ == Xptr::down<BooleanObject>(other)->value_;
}

void BooleanObject::blacken(VM& vm) {
}

BooleanObject* BooleanObject::create(VM& vm, bool b) {
  auto* o = new BooleanObject(b);
  vm.put_in(o);
  return o;
}

sz_t NumericObject::size(void) const {
  return sizeof(*this);
}

str_t NumericObject::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

bool NumericObject::is_equal(Object* other) const {
  return value_ == Xptr::down<NumericObject>(other)->value_;
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

StringObject::StringObject(StringObject* a, StringObject* b)
  : Object(ObjType::STRING) {
  count_ += a == nullptr ? 0 : a->count();
  count_ += b == nullptr ? 0 : b->count();
  chars_ = new char[count_ + 1];

  int offset = 0;
  if (a != nullptr) {
    memcpy(chars_, a->chars(), a->count());
    offset = a->count();
  }
  if (b != nullptr)
    memcpy(chars_ + offset, b->chars(), b->count());
  chars_[count_] = 0;
}

StringObject::~StringObject(void) {
  if (chars_ != nullptr)
    delete [] chars_;
}

sz_t StringObject::size(void) const {
  return sizeof(*this) + sizeof(char) * count_;
}

str_t StringObject::stringify(void) const {
  return chars_;
}

bool StringObject::is_equal(Object* other) const {
  auto* r = Xptr::down<StringObject>(other);
  return count_ == r->count_ && memcmp(chars_, r->chars_, count_) == 0;
}

void StringObject::blacken(VM&) {
}

StringObject* StringObject::create(VM& vm, const char* s, int n) {
  auto* o = new StringObject(s, n);
  vm.put_in(o);
  return o;
}

StringObject* StringObject::concat(VM& vm, StringObject* a, StringObject* b) {
  auto* o = new StringObject(a, b);
  vm.put_in(o);
  return o;
}

FunctionObject::FunctionObject(void)
  : Object(ObjType::FUNCTION) {
}

FunctionObject::~FunctionObject(void) {
  if (codes_ != nullptr)
    delete [] codes_;
}

int FunctionObject::dump_instruction(int i) {
  const auto* codes = codes_;
  fprintf(stdout, "%04d ", i);
  switch (codes[i++]) {
  case OpCode::OP_CONSTANT:
    {
      u8_t constant = codes[i++];
      fprintf(stdout, "%-16s %4d `", "OP_CONSTANT", constant);
      std::cout << get_constant(constant) << "`" << std::endl;
    } break;
  case OpCode::OP_POP: std::cout << "OP_POP" << std::endl; break;
  case OpCode::OP_DEF_GLOBAL:
    {
      u8_t name = codes[i++];
      fprintf(stdout, "%-16s %4d `", "OP_DEF_GLOBAL", name);
      std::cout << get_constant(name) << "`" << std::endl;
    } break;
  case OpCode::OP_GET_GLOBAL:
    {
      u8_t name = codes[i++];
      fprintf(stdout, "%-16s %4d `", "OP_GET_GLOBAL", name);
      std::cout << get_constant(name) << "`" << std::endl;
    } break;
  case OpCode::OP_SET_GLOBAL:
    {
      u8_t name = codes[i++];
      fprintf(stdout, "%-16s %4d `", "OP_SET_GLOBAL", name);
      std::cout << get_constant(name) << "`" << std::endl;
    } break;
  case OpCode::OP_GT: std::cout << "OP_GT" << std::endl; break;
  case OpCode::OP_GE: std::cout << "OP_GE" << std::endl; break;
  case OpCode::OP_LT: std::cout << "OP_LT" << std::endl; break;
  case OpCode::OP_LE: std::cout << "OP_LE" << std::endl; break;
  case OpCode::OP_ADD: std::cout << "OP_ADD" << std::endl; break;
  case OpCode::OP_SUB: std::cout << "OP_SUB" << std::endl; break;
  case OpCode::OP_MUL: std::cout << "OP_MUL" << std::endl; break;
  case OpCode::OP_DIV: std::cout << "OP_DIV" << std::endl; break;
  case OpCode::OP_NOT: std::cout << "OP_NOT" << std::endl; break;
  case OpCode::OP_NEG: std::cout << "OP_NEG" << std::endl; break;
  case OpCode::OP_RETURN: std::cout << "OP_RETURN" << std::endl; break;
  case OpCode::OP_JUMP:
    {
      u16_t offset = static_cast<u16_t>(codes[i++] << 8);
      offset |= codes[i++];
      fprintf(stdout, "%-16s %4d -> %d\n", "OP_JUMP", offset, i + offset);
    } break;
  case OpCode::OP_JUMP_IF_FALSE:
    {
      u16_t offset = static_cast<u16_t>(codes[i++] << 8);
      offset |= codes[i++];
      fprintf(stdout, "%-16s %4d -> %d\n", "OP_JUMP_IF_FALSE", offset, i + offset);
    } break;
  }
  return i;
}

void FunctionObject::dump(void) {
  for (int i = 0; i < codes_count_;) {
    i = dump_instruction(i);
  }
}

int FunctionObject::append_code(u8_t c) {
  if (codes_capacity_ < codes_count_ + 1) {
    codes_capacity_ = codes_capacity_ == 0 ? 4 : codes_capacity_ * 2;

    auto* new_codes = new u8_t[codes_capacity_];
    if (codes_ != nullptr) {
      memcpy(new_codes, codes_, sizeof(u8_t) * codes_count_);
      delete [] codes_;
    }
    codes_ = new_codes;
  }
  codes_[codes_count_] = c;
  return codes_count_++;
}

int FunctionObject::append_constant(Value v) {
  return constants_.append_value(v);
}

sz_t FunctionObject::size(void) const {
  return sizeof(*this);
}

str_t FunctionObject::stringify(void) const {
  // TODO:
  return "function";
}

bool FunctionObject::is_equal(Object*) const {
  return false;
}

void FunctionObject::blacken(VM& vm) {
  constants_.gray(vm);
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

void TableObject::set_entry(StringObject* key, Value val) {
  for (int i = 0; i < count_; ++i) {
    auto& entry = entries_[i];
    if (key->is_equal(entry.key)) {
      entry.value = val;
      return;
    }
  }

  if (capacity_ * kMaxLoad <= count_) {
    capacity_ = capacity_ == 0 ? 4 : capacity_ * 2;

    auto* new_entries = new TableEntry[capacity_];
    if (entries_ != nullptr) {
      memcpy(new_entries, entries_, sizeof(TableEntry) * count_);
      delete [] entries_;
    }
    entries_ = new_entries;
  }
  auto& entry = entries_[count_++];
  entry.key = key;
  entry.value = val;
}

Value TableObject::get_entry(StringObject* key) {
  for (int i = 0; i < count_; ++i) {
    auto& entry = entries_[i];
    if (key->is_equal(entry.key))
      return entry.value;
  }
  return nullptr;
}

sz_t TableObject::size(void) const {
  return sizeof(*this);
}

str_t TableObject::stringify(void) const {
  // TODO:
  return "table";
}

bool TableObject::is_equal(Object*) const {
  return false;
}

void TableObject::blacken(VM& vm) {
  for (int i = 0; i < count_; ++i) {
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
