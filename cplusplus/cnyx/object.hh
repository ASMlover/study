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

  PAIR, // NEED TO DELETE
};

class VM;

class Object : private UnCopyable {
  ObjType type_;
public:
  Object(ObjType t) : type_(t) {}
  virtual ~Object(void) {}

  inline ObjType get_type(void) const { return type_; }
  inline void set_type(ObjType t) { type_ = t; }
  inline void* address(void) const { return (void*)this; }
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
  virtual void traverse(VM* vm) = 0;
  virtual Object* move_to(void* p) = 0;
};
using Value = Object*;

std::ostream& operator<<(std::ostream& out, Object* o);

class Array : public Object {
  int count_{};
  Value* elements_{};
public:
  Array(void) : Object(ObjType::ARRAY) {}
  Array(int count);
  Array(Array&& r)
    : Object(ObjType::ARRAY)
    , count_(std::move(r.count_))
    , elements_(std::move(r.elements_)) {
  }

  inline int count(void) const { return count_; }
  inline Value* elements(void) const { return elements_; }
  inline Value get_element(int i) const { return elements_[i]; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM* vm) override;
  virtual Object* move_to(void* p) override;

  static Array* create(VM* vm, int count);
};

class Forward : public Object {
  Object* to_{};
public:
  Forward(void) : Object(ObjType::FORWARD) {}
  Forward(Forward&& r) : Object(ObjType::FORWARD), to_(std::move(r.to_)) {}

  inline void set_to(Object* v) { to_ = v; }
  inline Object* to(void) const { return to_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM* vm) override;
  virtual Object* move_to(void* p) override;

  static Forward* create(VM* vm);
};

class Function : public Object {
  Array* constants_{};
  int code_size_{};
  std::uint8_t* codes_{};
public:
  Function(void) : Object(ObjType::FUNCTION) {}
  Function(Array* constants, std::uint8_t* codes, int code_size);
  Function(Function&& r)
    : Object(ObjType::FUNCTION)
    , constants_(std::move(r.constants_))
    , code_size_(std::move(r.code_size_))
    , codes_(std::move(codes_)) {
  }

  inline Array* constants(void) const { return constants_; }
  inline Value get_constant(int i) const { return constants_->get_element(i); }
  inline int code_size(void) const { return code_size_; }
  inline const std::uint8_t* codes(void) const { return codes_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM* vm) override;
  virtual Object* move_to(void* p) override;

  static Function* create(VM* vm,
      Array* constants, std::uint8_t* codes, int code_size);
};

class Numeric : public Object {
  double value_{};
public:
  Numeric(void) : Object(ObjType::NUMERIC) {}
  Numeric(double d) : Object(ObjType::NUMERIC), value_(d) {}
  Numeric(Numeric&& r) : Object(ObjType::NUMERIC), value_(std::move(r.value_)) {}

  inline void set_value(double v) { value_ = v; }
  inline double value(void) const { return value_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM* vm) override;
  virtual Object* move_to(void* p) override;

  static Numeric* create(VM* vm, double d);
};

class String : public Object {
  int count_{};
  char* chars_{};
public:
  String(void) : Object(ObjType::STRING) {}
  String(const char* s, int n);
  String(String&& r)
    : Object(ObjType::STRING)
    , count_(std::move(r.count_))
    , chars_(std::move(r.chars_)) {
  }

  inline int count(void) const { return count_; }
  inline char* chars(void) const { return chars_; }
  inline char* c_str(void) const { return chars_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM* vm) override;
  virtual Object* move_to(void* p) override;

  static String* create(VM* vm, const char* s, int n);
};

struct TableEntry { Value key, value; };

class TableEntries : public Object {
  int count_{};
  TableEntry* entries_{};
public:
  TableEntries(void) : Object(ObjType::TABLE_ENTRIES) {}
  TableEntries(TableEntries&& r)
    : Object(ObjType::TABLE_ENTRIES)
    , count_(std::move(r.count_))
    , entries_(std::move(r.entries_)) {
  }

  inline int count(void) const { return count_; }
  inline TableEntry* entries(void) const { return entries_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM* vm) override;
  virtual Object* move_to(void* p) override;
};

class Table : public Object {
  int count_{};
  TableEntries* entries_{};
public:
  Table(void) : Object(ObjType::TABLE) {}
  Table(Table&& r)
    : Object(ObjType::TABLE)
    , count_(std::move(r.count_))
    , entries_(std::move(r.entries_)) {
  }

  inline int count(void) const { return count_; }
  inline TableEntries* entries(void) const { return entries_; }

  virtual std::size_t size(void) const override;
  virtual std::string stringify(void) const override;
  virtual void traverse(VM* vm) override;
  virtual Object* move_to(void* p) override;

  static Table* create(VM* vm);
};

}
