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

sz_t BooleanObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t BooleanObject::stringify(void) const {
  return value_ ? "true" : "false";
}

bool BooleanObject::is_equal(BaseObject* other) const {
  return value_ == Xptr::down<BooleanObject>(other)->value_;
}

void BooleanObject::blacken(VM& vm) {
}

BooleanObject* BooleanObject::create(VM& vm, bool b) {
  auto* o = new BooleanObject(b);
  vm.append_object(o);
  return o;
}

sz_t NumericObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t NumericObject::stringify(void) const {
  std::stringstream ss;
  ss << value_;
  return ss.str();
}

bool NumericObject::is_equal(BaseObject* other) const {
  return value_ == Xptr::down<NumericObject>(other)->value_;
}

void NumericObject::blacken(VM&) {
}

NumericObject* NumericObject::create(VM& vm, double d) {
  auto* o = new NumericObject(d);
  vm.append_object(o);
  return o;
}

StringObject::StringObject(const char* s, int n)
  : BaseObject(ObjType::STRING)
  , count_(n) {
  chars_ = new char[count_ + 1];
  if (s != nullptr)
    memcpy(chars_, s, n);
  chars_[count_] = 0;
}

StringObject::StringObject(StringObject* a, StringObject* b)
  : BaseObject(ObjType::STRING) {
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

sz_t StringObject::size_bytes(void) const {
  return sizeof(*this) + sizeof(char) * count_;
}

str_t StringObject::stringify(void) const {
  return chars_;
}

bool StringObject::is_equal(BaseObject* other) const {
  auto* r = Xptr::down<StringObject>(other);
  return count_ == r->count_ && memcmp(chars_, r->chars_, count_) == 0;
}

void StringObject::blacken(VM&) {
}

StringObject* StringObject::create(VM& vm, const char* s, int n) {
  auto* o = new StringObject(s, n);
  vm.append_object(o);
  return o;
}

StringObject* StringObject::concat(VM& vm, StringObject* a, StringObject* b) {
  auto* o = new StringObject(a, b);
  vm.append_object(o);
  return o;
}

FunctionObject::FunctionObject(void)
  : BaseObject(ObjType::FUNCTION) {
}

FunctionObject::~FunctionObject(void) {
  if (codes_ != nullptr)
    delete [] codes_;
  if (codelines_ != nullptr)
    delete [] codelines_;
}

int FunctionObject::dump_instruction(int i) {
  static auto constant_instruction =
    [](FunctionObject* fn, int i, const char* name) -> int {
      u8_t constant = fn->get_code(i++);
      fprintf(stdout, "%-16s %4d ", name, constant);
      std::cout << "`" << fn->get_constant(constant) << "`" << std::endl;
      return i;
    };

  const auto* codes = codes_;
  fprintf(stdout, "%04d ", i);
  if (i > 0 && codelines_[i] == codelines_[i - 1])
    std::cout << "  | ";
  else
    fprintf(stdout, "%3d ", codelines_[i]);

  switch (auto instruction = codes[i++]; instruction) {
  case OpCode::OP_CONSTANT:
    i = constant_instruction(this, i, "OP_CONSTANT"); break;
  case OpCode::OP_NIL: std::cout << "OP_NIL" << std::endl; break;
  case OpCode::OP_POP: std::cout << "OP_POP" << std::endl; break;
  case OpCode::OP_GET_LOCAL:
    {
      u8_t slot = codes[i++];
      fprintf(stdout, "%-16s %4d\n", "OP_GET_LOCAL", slot);
    } break;
  case OpCode::OP_SET_LOCAL:
    {
      u8_t slot = codes[i++];
      fprintf(stdout, "%-16s %4d\n", "OP_SET_LOCAL", slot);
    } break;
  case OpCode::OP_DEF_GLOBAL:
    i = constant_instruction(this, i, "OP_DEF_GLOBAL"); break;
  case OpCode::OP_GET_GLOBAL:
    i = constant_instruction(this, i, "OP_GET_GLOBAL"); break;
  case OpCode::OP_SET_GLOBAL:
    i = constant_instruction(this, i, "OP_SET_GLOBAL"); break;
  case OpCode::OP_GET_UPVALUE:
    fprintf(stdout, "%-16s %4d\n", "OP_GET_UPVALUE", codes_[i++]); break;
  case OpCode::OP_SET_UPVALUE:
    fprintf(stdout, "%-16s %4d\n", "OP_SET_UPVALUE", codes_[i++]); break;
  case OpCode::OP_GET_FIELD:
    i = constant_instruction(this, i, "OP_GET_FIELD"); break;
  case OpCode::OP_SET_FIELD:
    i = constant_instruction(this, i, "OP_SET_FIELD"); break;
  case OpCode::OP_EQ: std::cout << "OP_EQ" << std::endl; break;
  case OpCode::OP_NE: std::cout << "OP_NE" << std::endl; break;
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
  case OpCode::OP_LOOP:
    {
      u16_t offset = static_cast<u16_t>(codes[i++] << 8);
      offset |= codes[i++];
      fprintf(stdout, "%-16s %4d -> %d\n", "OP_LOOP", offset, i - offset);
    } break;
  case OpCode::OP_CALL_0:
  case OpCode::OP_CALL_1:
  case OpCode::OP_CALL_2:
  case OpCode::OP_CALL_3:
  case OpCode::OP_CALL_4:
  case OpCode::OP_CALL_5:
  case OpCode::OP_CALL_6:
  case OpCode::OP_CALL_7:
  case OpCode::OP_CALL_8:
    std::cout << "OP_CALL_" << instruction - OpCode::OP_CALL_0 << std::endl;
    break;
  case OpCode::OP_CLOSURE:
    {
      u8_t constant = codes_[i++];
      auto* constant_value = constants_.get_value(constant);
      fprintf(stdout, "%-16s %4d ", "OP_CLOSURE", constant);
      std::cout << constant_value << std::endl;

      auto* closed_fn = Xptr::down<FunctionObject>(constant_value);
      for (int j = 0; j < closed_fn->upvalues_count(); ++j) {
        u8_t is_local = codes_[i++];
        u8_t index = codes_[i++];
        fprintf(stdout, "%04d   |                     %s %d\n",
            i - 2, is_local ? "local" : "upvalue", index);
      }
    } break;
  case OpCode::OP_CLOSE_UPVALUE: std::cout << "OP_CLOSE_UPVALUE" << std::endl; break;
  case OpCode::OP_RETURN: std::cout << "OP_RETURN" << std::endl; break;
  case OpCode::OP_CLASS:
    i = constant_instruction(this, i, "OP_CLASS"); break;
  case OpCode::OP_METHOD:
    i = constant_instruction(this, i, "OP_METHOD"); break;
  }
  return i;
}

void FunctionObject::dump(void) {
  std::cout << "---------" << std::endl;
  for (int i = 0; i < codes_count_;) {
    i = dump_instruction(i);
  }
}

int FunctionObject::append_code(u8_t c, int lineno) {
  if (codes_capacity_ < codes_count_ + 1) {
    codes_capacity_ = codes_capacity_ == 0 ? 4 : codes_capacity_ * 2;

    auto* new_codes = new u8_t[codes_capacity_];
    if (codes_ != nullptr) {
      memcpy(new_codes, codes_, sizeof(u8_t) * codes_count_);
      delete [] codes_;
    }
    codes_ = new_codes;

    auto* new_codelines = new int[codes_capacity_];
    if (codelines_ != nullptr) {
      memcpy(new_codelines, codelines_, sizeof(int) * codes_count_);
      delete [] codelines_;
    }
    codelines_ = new_codelines;
  }
  codes_[codes_count_] = c;
  codelines_[codes_count_] = lineno;

  return codes_count_++;
}

int FunctionObject::append_constant(Value v) {
  return constants_.append_value(v);
}

sz_t FunctionObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t FunctionObject::stringify(void) const {
  std::stringstream ss;
  ss << "<fn `" << this << "`>";
  return ss.str();
}

bool FunctionObject::is_equal(BaseObject*) const {
  return false;
}

void FunctionObject::blacken(VM& vm) {
  constants_.gray(vm);
}

FunctionObject* FunctionObject::create(VM& vm) {
  auto* o = new FunctionObject();
  vm.append_object(o);
  return o;
}

TableObject::~TableObject(void) {
  if (entries_ != nullptr)
    delete [] entries_;
}

bool TableObject::set_entry(StringObject* key, Value val) {
  for (int i = 0; i < count_; ++i) {
    auto& entry = entries_[i];
    if (key->is_equal(entry.key)) {
      entry.value = val;
      return true;
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

  return false;
}

std::optional<Value> TableObject::get_entry(StringObject* key) const {
  for (int i = 0; i < count_; ++i) {
    auto& entry = entries_[i];
    if (key->is_equal(entry.key))
      return {entry.value};
  }
  return {};
}

sz_t TableObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t TableObject::stringify(void) const {
  // TODO:
  return "table";
}

bool TableObject::is_equal(BaseObject*) const {
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
  return "upvalue";
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
  , function_(fn) {
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
  return "closure";
}

bool ClosureObject::is_equal(BaseObject* other) const {
  return false;
}

void ClosureObject::blacken(VM& vm) {
  vm.gray_value(function_);
  for (int i = 0; i < function_->upvalues_count(); ++i)
    vm.gray_value(upvalues_[i]);
}

ClosureObject* ClosureObject::create(VM& vm, FunctionObject* fn) {
  auto* o = new ClosureObject(fn);
  vm.append_object(o);
  return o;
}

ClassObject::ClassObject(StringObject* name, Value superclass)
  : BaseObject(ObjType::CLASS)
  , name_(name) {
}

ClassObject::~ClassObject(void) {
}

sz_t ClassObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t ClassObject::stringify(void) const {
  return name_ != nullptr ? name_->chars() : "class";
}

bool ClassObject::is_equal(BaseObject* other) const {
  return false;
}

void ClassObject::blacken(VM& vm) {
  vm.gray_value(name_);
  vm.gray_value(methods_);
}

ClassObject* ClassObject::create(VM& vm, StringObject* name, Value superclass) {
  auto* o = new ClassObject(name, superclass);
  vm.append_object(o);
  return nullptr;
}

InstanceObject::InstanceObject(ClassObject* klass, TableObject* fields)
  : BaseObject(ObjType::INSTANCE) , class_(klass), fields_(fields) {
}

InstanceObject::~InstanceObject(void) {
}

sz_t InstanceObject::size_bytes(void) const {
  return sizeof(*this);
}

str_t InstanceObject::stringify(void) const {
  std::stringstream ss;
  ss << "<" << class_->stringify() << " instance>";
  return ss.str();
}

bool InstanceObject::is_equal(BaseObject* other) const {
  return false;
}

void InstanceObject::blacken(VM& vm) {
  vm.gray_value(class_);
  vm.gray_value(fields_);
}

InstanceObject* InstanceObject::create(VM& vm, ClassObject* klass) {
  auto* o = new InstanceObject(klass, TableObject::create(vm));
  vm.append_object(o);
  return o;
}

}
