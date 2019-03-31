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
  ARRAY,
  FORWARD,
  FUNCTION,
  NUMERIC,
  STRING,
  TABLE,
  TABLE_ENTRIES,
};

class VM;

class Object : private UnCopyable {
  ObjType type_;
public:
  Object(ObjType t) : type_(t) {}
  virtual ~Object(void) {}

  inline ObjType get_type(void) const { return type_; }
  inline void set_type(ObjType t) { type_ = t; }
  inline void* address(void) { return reinterpret_cast<void*>(this); }
  inline const void* address(void) const { return reinterpret_cast<const void*>(this); }
  inline byte_t* as_byte(void) { return reinterpret_cast<byte_t*>(this); }

  template <typename Target> inline Target* cast_to(void) {
    return static_cast<Target*>(this);
  }

  template <typename Target> inline Target* down_to(void) {
    // this must be Object type
    return dynamic_cast<Target*>(this);
  }

  virtual std::size_t size(void) const = 0;
  virtual std::string stringify(void) const = 0;
  virtual void traverse(VM& vm) = 0;
  virtual Object* move_to(void* p) = 0;
};
using Value = Object*;

std::ostream& operator<<(std::ostream& out, Object* o);

class ArrayObject : public Object {
  int count_{};
  Value* elements_{};

  ArrayObject(int count);
  ArrayObject(ArrayObject&& r)
    : Object(ObjType::ARRAY)
    , count_(std::move(r.count_))
    , elements_(std::move(r.elements_)) {
  }
public:
  inline int count(void) const { return count_; }
  inline Value* elements(void) { return elements_; }
  inline const Value* elements(void) const { return elements_; }
  inline Value get_element(int i) { return elements_[i]; }
  inline const Value get_element(int i) const { return elements_[i]; }
  inline void set_element(int i, Value v) { elements_[i] = v; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM& vm) override;
  virtual Object* move_to(void* p) override;

  static ArrayObject* create(VM& vm, int count);
  static ArrayObject* ensure(VM& vm, ArrayObject* orig_array, int new_count);
};

class ForwardObject : public Object {
  Object* to_{};

  ForwardObject(void) : Object(ObjType::FORWARD) {}
  ForwardObject(ForwardObject&& r) : Object(ObjType::FORWARD), to_(std::move(r.to_)) {}
public:
  inline void set_to(Object* v) { to_ = v; }
  inline Object* to(void) { return to_; }
  inline const Object* to(void) const { return to_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM& vm) override;
  virtual Object* move_to(void* p) override;

  static ForwardObject* forward(void* p);
  static ForwardObject* create(VM& vm);
};

class FunctionObject : public Object {
  ArrayObject* constants_{};
  int code_size_{};
  std::uint8_t* codes_{};

  FunctionObject(const ArrayObject* constants,
      const std::uint8_t* codes, int code_size);
  FunctionObject(FunctionObject&& r)
    : Object(ObjType::FUNCTION)
    , constants_(std::move(r.constants_))
    , code_size_(std::move(r.code_size_))
    , codes_(std::move(codes_)) {
  }
public:
  inline ArrayObject* constants(void) { return constants_; }
  inline const ArrayObject* constants(void) const { return constants_; }
  inline Value get_constant(int i) { return constants_->get_element(i); }
  inline const Value get_constant(int i) const { return constants_->get_element(i); }
  inline int code_size(void) const { return code_size_; }
  inline std::uint8_t* codes(void) { return codes_; }
  inline const std::uint8_t* codes(void) const { return codes_; }
  void dump(void);

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM& vm) override;
  virtual Object* move_to(void* p) override;

  static FunctionObject* create(VM& vm,
      const ArrayObject* constants, const std::uint8_t* codes, int code_size);
};

class NumericObject : public Object {
  double value_{};

  NumericObject(double d) : Object(ObjType::NUMERIC), value_(d) {}
  NumericObject(NumericObject&& r)
    : Object(ObjType::NUMERIC), value_(std::move(r.value_)) {
  }
public:
  inline void set_value(double v) { value_ = v; }
  inline double value(void) const { return value_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM& vm) override;
  virtual Object* move_to(void* p) override;

  static NumericObject* create(VM& vm, double d);
};

class StringObject : public Object {
  int count_{};
  char* chars_{};

  StringObject(const char* s, int n);
  StringObject(StringObject&& r)
    : Object(ObjType::STRING)
    , count_(std::move(r.count_))
    , chars_(std::move(r.chars_)) {
  }
public:
  inline int count(void) const { return count_; }
  inline char* chars(void) { return chars_; }
  inline const char* chars(void) const { return chars_; }
  inline const char* c_str(void) const { return chars_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM& vm) override;
  virtual Object* move_to(void* p) override;

  static StringObject* create(VM& vm, const char* s, int n);
};

struct TableEntry { Value key, value; };

class TableEntriesObject : public Object {
  int count_{};
  TableEntry* entries_{};

  TableEntriesObject(void) : Object(ObjType::TABLE_ENTRIES) {}
  TableEntriesObject(TableEntriesObject&& r)
    : Object(ObjType::TABLE_ENTRIES)
    , count_(std::move(r.count_))
    , entries_(std::move(r.entries_)) {
  }
public:
  inline int count(void) const { return count_; }
  inline TableEntry* entries(void) { return entries_; }
  inline const TableEntry* entries(void) const { return entries_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM& vm) override;
  virtual Object* move_to(void* p) override;
};

class TableObject : public Object {
  int count_{};
  TableEntriesObject* entries_{};

  TableObject(void) : Object(ObjType::TABLE) {}
  TableObject(TableObject&& r)
    : Object(ObjType::TABLE)
    , count_(std::move(r.count_))
    , entries_(std::move(r.entries_)) {
  }
public:
  inline int count(void) const { return count_; }
  inline TableEntriesObject* entries(void) { return entries_; }
  inline const TableEntriesObject* entries(void) const { return entries_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM& vm) override;
  virtual Object* move_to(void* p) override;

  static TableObject* create(VM& vm);
};

}
