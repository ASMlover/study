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

#include <functional>
#include <vector>
#include "common.hh"

namespace wrencc {

enum class ValueType {
  NIL,
  TRUE,
  FALSE,
  NUMERIC,
  OBJECT,
};

enum class ObjType {
  STRING,
  FUNCTION,
  CLASS,
  INSTANCE,
};

enum ObjFlag {
  MARKED = 0x01,
};

class VM;
class BaseObject;

class NilObject;
class BooleanObject;
class NumericObject;
class StringObject;
class FunctionObject;
class ClassObject;
class InstanceObject;

class BaseObject : private UnCopyable {
  ObjType type_;
  ObjFlag flag_{};
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject(void) {}

  inline ObjType type(void) const { return type_; }
  inline ObjFlag flag(void) const { return flag_; }
  template <typename T> inline void set_flag(T f) { flag_ = Xt::as_type<ObjFlag>(f); }

  virtual str_t stringify(void) const = 0;
  virtual void gc_mark(VM& vm) {}
};

class Value : public Copyable {
  ValueType type_{};
  double num_{};
  BaseObject* obj_{};

  inline bool check(ObjType type) const { return is_object() && obj_->type() == type; }

  template <typename T> inline double to_decimal(T x) {
    return Xt::as_type<double>(x);
  }
public:
  Value(void) noexcept : type_(ValueType::OBJECT) {}
  Value(nil_t) noexcept : type_(ValueType::NIL) {}
  Value(bool b) noexcept : type_(b ? ValueType::TRUE : ValueType::FALSE) {}
  Value(i8_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(u8_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(i16_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(u16_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(i32_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(u32_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(i64_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(u64_t n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(float n) noexcept : type_(ValueType::NUMERIC), num_(to_decimal(n)) {}
  Value(double d) noexcept : type_(ValueType::NUMERIC), num_(d) {}
  Value(BaseObject* obj) noexcept : type_(ValueType::OBJECT), obj_(obj) {}

  inline ValueType type(void) const { return type_; }
  inline ObjType objtype(void) const { return obj_->type(); }
  inline bool is_valid(void) const { return type_ != ValueType::OBJECT || obj_ != nullptr; }

  inline bool is_nil(void) const { return type_ == ValueType::NIL; }
  inline bool is_boolean(void) const { return type_ == ValueType::TRUE || type_ == ValueType::FALSE; }
  inline bool is_numeric(void) const { return type_ == ValueType::NUMERIC; }
  inline bool is_object(void) const { return type_ == ValueType::OBJECT; }
  inline bool is_string(void) const { return check(ObjType::STRING); }
  inline bool is_function(void) const { return check(ObjType::FUNCTION); }
  inline bool is_class(void) const { return check(ObjType::CLASS); }
  inline bool is_instance(void) const { return check(ObjType::INSTANCE); }

  inline BaseObject* as_object(void) const { return obj_; }
  inline bool as_boolean(void) const { return type_ == ValueType::TRUE; }
  inline double as_numeric(void) const { return num_; }

  StringObject* as_string(void) const;
  const char* as_cstring(void) const;
  FunctionObject* as_function(void) const;
  ClassObject* as_class(void) const;
  InstanceObject* as_instance(void) const;

  str_t stringify(void) const;

  static Value no_value(void) {
    return Value();
  }
};

std::ostream& operator<<(std::ostream& out, const Value& val);

class StringObject final : public BaseObject {
  int size_{};
  char* value_{};

  StringObject(const char* s, int n, bool replace_owner = false) noexcept;
  virtual ~StringObject(void);
public:
  inline int size(void) const { return size_; }
  inline const char* cstr(void) const { return value_; }

  virtual str_t stringify(void) const override;

  static StringObject* make_string(VM& vm, const char* s, int n);
  static StringObject* make_string(VM& vm, const str_t& s);
  static StringObject* make_string(VM& vm, StringObject* s1, StringObject* s2);
};

class FunctionObject final : public BaseObject {
  std::vector<u8_t> codes_;
  std::vector<Value> constants_;

  FunctionObject(void) noexcept : BaseObject(ObjType::FUNCTION) {}
public:
  inline const u8_t* codes(void) const { return codes_.data(); }
  inline const Value* constants(void) const { return constants_.data(); }
  inline int codes_count(void) const { return Xt::as_type<int>(codes_.size()); }
  inline int constants_count(void) const { return Xt::as_type<int>(constants_.size()); }
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline const Value& get_constant(int i) const { return constants_[i]; }

  template <typename T> inline int add_code(T c) {
    codes_.push_back(Xt::as_type<u8_t>(c));
    return Xt::as_type<int>(codes_.size()) - 1;
  }

  template <typename T> inline void set_code(int i, T c) {
    codes_[i] = Xt::as_type<u8_t>(c);
  }

  inline int add_constant(const Value& v) {
    constants_.push_back(v);
    return Xt::as_type<int>(constants_.size()) - 1;
  }

  virtual str_t stringify(void) const override;
  virtual void gc_mark(VM& vm) override;

  static FunctionObject* make_function(VM& vm);
};

class Fiber;
using PrimitiveFn = Value (*)(VM& vm, Value* args);
using FiberPrimitiveFn = void (*)(VM& vm, Fiber& fiber, Value* args);

enum class MethodType {
  NONE,     // no method for the given symbol
  PRIMITIVE,// a primitive method implemented in C that immediatelt returns a Value
  FIBER,    // a built-in method that modifies the fiber directly
  BLOCK,    // a normal user-defined method
};

struct Method {
  MethodType type{MethodType::NONE};
  union {
    PrimitiveFn primitive;
    FiberPrimitiveFn fiber_primitive;
    FunctionObject* fn;
  };
};

class ClassObject final : public BaseObject {
  static constexpr sz_t kMaxMethods = 256;

  ClassObject* meta_class_{};
  ClassObject* superclass_{};
  std::vector<Method> methods_{kMaxMethods};

  ClassObject(void) noexcept;
  ClassObject(ClassObject* meta_class, ClassObject* supercls = nullptr) noexcept;
public:
  inline ClassObject* meta_class(void) const { return meta_class_; }
  inline int methods_count(void) const { return Xt::as_type<int>(methods_.size()); }
  inline Method& get_method(int i) { return methods_[i]; }
  inline void set_method(int i, PrimitiveFn fn) {
    methods_[i].type = MethodType::PRIMITIVE;
    methods_[i].primitive = fn;
  }
  inline void set_method(int i, FiberPrimitiveFn fn) {
    methods_[i].type = MethodType::FIBER;
    methods_[i].fiber_primitive = fn;
  }
  inline void set_method(int i, FunctionObject* fn) {
    methods_[i].type = MethodType::BLOCK;
    methods_[i].fn = fn;
  }

  virtual str_t stringify(void) const override;
  virtual void gc_mark(VM& vm) override;

  static ClassObject* make_class(VM& vm, ClassObject* superclass = nullptr);
};

class InstanceObject final : public BaseObject {
  ClassObject* cls_{};
  // TODO: need add instance fields

  InstanceObject(ClassObject* cls) noexcept;
public:
  inline ClassObject* cls(void) const { return cls_; }

  virtual str_t stringify(void) const override;

  static InstanceObject* make_instance(VM& vm, ClassObject* cls);
};

}
