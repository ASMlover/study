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
  case ObjType::BOOLEAN: return "<boolean>";
  case ObjType::NUMERIC: return "<numeric>";
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
  return out << (obj == nullptr ? "nil" : obj->stringify());
}

void gray_table(VM& vm, table_t& tbl) {
  for (auto& t : tbl)
    vm.gray_value(t.second);
}

void remove_table_undark(table_t& tbl) {
  for (auto it = tbl.begin(); it != tbl.end();) {
    if (!it->second.as_obj()->is_dark())
      tbl.erase(it++);
    else
      ++it;
  }
}

ValueArray::~ValueArray(void) {
  if (values_ != nullptr)
    delete [] values_;
}

void ValueArray::set_value(int i, const Value& v) {
  values_[i] = v;
}

Value& ValueArray::get_value(int i) {
  return values_[i];
}

const Value& ValueArray::get_value(int i) const {
  return values_[i];
}

int ValueArray::append_value(const Value& v) {
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
  return value_ == Xptr::down<BooleanObject>(other)->value();
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
  return value_ == Xptr::down<NumericObject>(other)->value();
}

void NumericObject::blacken(VM&) {
}

NumericObject* NumericObject::create(VM& vm, double d) {
  auto* o = new NumericObject(d);
  vm.append_object(o);
  return o;
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
  case OpCode::OP_GET_SUPER:
    i = constant_instruction(this, i, "OP_GET_SUPER"); break;
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
  case OpCode::OP_INVOKE_0:
    i = constant_instruction(this, i, "OP_INVOKE_0"); break;
  case OpCode::OP_INVOKE_1:
    i = constant_instruction(this, i, "OP_INVOKE_1"); break;
  case OpCode::OP_INVOKE_2:
    i = constant_instruction(this, i, "OP_INVOKE_2"); break;
  case OpCode::OP_INVOKE_3:
    i = constant_instruction(this, i, "OP_INVOKE_3"); break;
  case OpCode::OP_INVOKE_4:
    i = constant_instruction(this, i, "OP_INVOKE_4"); break;
  case OpCode::OP_INVOKE_5:
    i = constant_instruction(this, i, "OP_INVOKE_5"); break;
  case OpCode::OP_INVOKE_6:
    i = constant_instruction(this, i, "OP_INVOKE_6"); break;
  case OpCode::OP_INVOKE_7:
    i = constant_instruction(this, i, "OP_INVOKE_7"); break;
  case OpCode::OP_INVOKE_8:
    i = constant_instruction(this, i, "OP_INVOKE_8"); break;
  case OpCode::OP_SUPER_0:
    i = constant_instruction(this, i, "OP_SUPER_0"); break;
  case OpCode::OP_SUPER_1:
    i = constant_instruction(this, i, "OP_SUPER_1"); break;
  case OpCode::OP_SUPER_2:
    i = constant_instruction(this, i, "OP_SUPER_2"); break;
  case OpCode::OP_SUPER_3:
    i = constant_instruction(this, i, "OP_SUPER_3"); break;
  case OpCode::OP_SUPER_4:
    i = constant_instruction(this, i, "OP_SUPER_4"); break;
  case OpCode::OP_SUPER_5:
    i = constant_instruction(this, i, "OP_SUPER_5"); break;
  case OpCode::OP_SUPER_6:
    i = constant_instruction(this, i, "OP_SUPER_6"); break;
  case OpCode::OP_SUPER_7:
    i = constant_instruction(this, i, "OP_SUPER_7"); break;
  case OpCode::OP_SUPER_8:
    i = constant_instruction(this, i, "OP_SUPER_8"); break;
  case OpCode::OP_CLOSURE:
    {
      u8_t constant = codes_[i++];
      auto& constant_value = constants_.get_value(constant);
      fprintf(stdout, "%-16s %4d ", "OP_CLOSURE", constant);
      std::cout << constant_value << std::endl;

      auto* closed_fn = constant_value.as_function();
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
  case OpCode::OP_SUBCLASS:
    i = constant_instruction(this, i, "OP_SUBCLASS"); break;
  case OpCode::OP_METHOD:
    i = constant_instruction(this, i, "OP_METHOD"); break;
  }
  return i;
}

void FunctionObject::dump(void) {
  std::cout << "--- [" << name_->chars() << "] ---" << std::endl;
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

int FunctionObject::append_constant(const Value& v) {
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
  std::stringstream ss;
  ss << "<fn `" << this << "`>";
  return ss.str();
}

bool ClosureObject::is_equal(BaseObject* other) const {
  return false;
}

void ClosureObject::blacken(VM& vm) {
  vm.gray_object(function_);
  for (int i = 0; i < function_->upvalues_count(); ++i)
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
  return name_ != nullptr ? name_->chars() : "class";
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
  ss << "<`" << class_->stringify() << "` instance>";
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
  ss << "<fn `" << this << "`";
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
