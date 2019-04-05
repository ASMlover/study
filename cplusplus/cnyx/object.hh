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
#pragma once

#include <cstdint>
#include <string>
#include "common.hh"

namespace nyx {

enum class ObjType {
  BOOLEAN,
  NUMERIC,
  STRING,
  FUNCTION,
  TABLE,
};

class VM;

class BaseObject : private UnCopyable {
  ObjType type_;
  bool is_dark_{};
public:
  BaseObject(ObjType t) : type_(t) {}
  virtual ~BaseObject(void) {}

  inline ObjType type(void) const { return type_; }
  inline void set_type(ObjType t) { type_ = t; }
  inline bool is_dark(void) const { return is_dark_; }
  inline void set_dark(bool is_dark) { is_dark_ = is_dark; }

  virtual sz_t size_bytes(void) const = 0;
  virtual str_t stringify(void) const = 0;
  virtual bool is_equal(BaseObject* other) const = 0;
  virtual void blacken(VM& vm) = 0;
};
using Value = BaseObject*;

inline std::ostream& operator<<(std::ostream& out, BaseObject* obj) {
  return out << obj->stringify();
}

inline bool values_equal(Value a, Value b) {
  if (a == b)
    return true;
  return a->type() != b->type() ? false : a->is_equal(b);
}

class ValueArray : private UnCopyable {
  int capacity_{};
  int count_{};
  Value* values_{};
public:
  ValueArray(void) {}
  ~ValueArray(void);

  inline int capacity(void) const { return capacity_; }
  inline int count(void) const { return count_; }
  inline Value* values(void) { return values_; }
  inline const Value* values(void) const { return values_; }
  inline void set_value(int i, Value v) { values_[i] = v; }
  inline Value get_value(int i) { return values_[i]; }
  inline const Value get_value(int i) const { return values_[i]; }
  int append_value(Value v);
  void gray(VM& vm);
};

class BooleanObject : public BaseObject {
  bool value_{};

  BooleanObject(bool b) : BaseObject(ObjType::BOOLEAN), value_(b) {}
  ~BooleanObject(void) {}
public:
  inline void set_value(bool b) { value_ = b; }
  inline bool value(void) const { return value_; }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static BooleanObject* create(VM& vm, bool b);
};

class NumericObject : public BaseObject {
  double value_{};

  NumericObject(double d) : BaseObject(ObjType::NUMERIC), value_(d) {}
  ~NumericObject(void) {}
public:
  inline void set_value(double v) { value_ = v; }
  inline double value(void) const { return value_; }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static NumericObject* create(VM& vm, double d);
};

class StringObject : public BaseObject {
  int count_{};
  char* chars_{};

  StringObject(const char* s, int n);
  StringObject(StringObject* a, StringObject* b);
  ~StringObject(void);
public:
  inline int count(void) const { return count_; }
  inline char* chars(void) { return chars_; }
  inline const char* chars(void) const { return chars_; }
  inline char get_element(int i) const { return chars_[i]; }
  inline void set_element(int i, char c) { chars_[i] = c; }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static StringObject* create(VM& vm, const char* s, int n);
  static StringObject* concat(VM& vm, StringObject* a, StringObject* b);
};

class FunctionObject : public BaseObject {
  int codes_capacity_{};
  int codes_count_{};
  u8_t* codes_{};

  ValueArray constants_;

  FunctionObject(void);
  ~FunctionObject(void);
public:
  inline int codes_capacity(void) const { return codes_capacity_; }
  inline int codes_count(void) const { return codes_count_; }
  inline u8_t* codes(void) { return codes_; }
  inline const u8_t* codes(void) const { return codes_; }
  inline void set_code(int i, u8_t c) { codes_[i] = c; }
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline int constants_capacity(void) const { return constants_.capacity(); }
  inline int constants_count(void) const { return constants_.count(); }
  inline Value* constants(void) { return constants_.values(); }
  inline const Value* constants(void) const { return constants_.values(); }
  inline void set_constant(int i, Value v) { constants_.set_value(i, v); }
  inline Value get_constant(int i) const { return constants_.get_value(i); }

  int dump_instruction(int i);
  void dump(void);
  int append_code(u8_t c);
  int append_constant(Value v);

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static FunctionObject* create(VM& vm);
};

class TableObject : public BaseObject {
public:
  struct TableEntry {
    StringObject* key;
    Value value;
  };
private:
  int capacity_{};
  int count_{};
  TableEntry* entries_{};

  static constexpr double kMaxLoad = 0.75;

  TableObject(void) : BaseObject(ObjType::TABLE) {}
  ~TableObject(void);
public:
  inline int capacity(void) const { return capacity_; }
  inline int count(void) const { return count_; }
  inline TableEntry* entries(void) { return entries_; }
  inline const TableEntry* entries(void) const { return entries_; }
  void set_entry(StringObject* key, Value value);
  Value get_entry(StringObject* key);

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static TableObject* create(VM& vm);
};

}
