// Copyright (c) 2020 ASMlover. All rights reserved.
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

#include <vector>
#include "common.hh"

namespace wrencc {

class WrenVM;

enum class ValueType : u8_t {
  UNDEFINED,

  NIL,
  TRUE,
  FALSE,
  NUMERIC,
  OBJECT,
};

enum class ObjType : u8_t {
  STRING,
  LIST,
  RANGE,
  MAP,
  MODULE,
  FUNCTION,
  FOREIGN,
  UPVALUE,
  CLOSURE,
  FIBER,
  CLASS,
  INSTANCE,
};

class StringObject;
class ListObject;
class RangeObject;
class MapObject;
class ModuleObject;
class FunctionObject;
class ForeignObject;
class UpvalueObject;
class ClosureObject;
class FiberObject;
class ClassObject;
class InstanceObject;

struct BaseObject : private UnCopyable {
  ObjType type_;
  bool is_darken_{};
  ClassObject* cls_{};
public:
  BaseObject(ObjType type, ClassObject* cls = nullptr) noexcept
    : type_(type), cls_(cls) {
  }
  virtual ~BaseObject() noexcept {}

  inline ObjType type() const noexcept { return type_; }
  inline bool is_darken() const noexcept { return is_darken_; }
  inline void set_darken(bool darken = true) noexcept { is_darken_ = darken; }
  inline ClassObject* cls() const noexcept { return cls_; }
  inline void set_cls(ClassObject* cls) noexcept { cls_ = cls; }

  StringObject* as_string() noexcept;
  const char* as_cstring() noexcept;
  ListObject* as_list() noexcept;
  RangeObject* as_range() noexcept;
  MapObject* as_map() noexcept;
  ModuleObject* as_module() noexcept;
  FunctionObject* as_function() noexcept;
  ForeignObject* as_foreign() noexcept;
  UpvalueObject* as_upvalue() noexcept;
  ClosureObject* as_closure() noexcept;
  FiberObject* as_fiber() noexcept;
  ClassObject* as_class() noexcept;
  InstanceObject* as_instance() noexcept;

  virtual bool is_equal(BaseObject* r) const { return false; }
  virtual str_t stringify() const { return "<object>"; }
  virtual void gc_blacken(WrenVM& vm) {}
  virtual void initialize(WrenVM& vm) {}
  virtual void finalize(WrenVM& vm) {}

  virtual u32_t hasher() const {
    ASSERT(false, "only immutable objects can be hashed");
    return 0;
  }
};

class ObjValue final : public Copyable {
  ValueType type_{ValueType::UNDEFINED};

  union {
    double num{};
    BaseObject* obj;
  } as_;

  inline bool is(ObjType type) const noexcept {
    return is_object() && objtype() == type;
  }

  inline bool is_true() const noexcept { return type_ == ValueType::TRUE; }
  inline bool is_false() const noexcept { return type_ == ValueType::FALSE; }

  template <typename T>
  inline void set_decimal(T x) { as_.num = Xt::to_decimal(x); }
public:
  ObjValue() noexcept {}
  ObjValue(nil_t) noexcept : type_(ValueType::NIL) {}
  ObjValue(bool b) noexcept : type_(b ? ValueType::TRUE : ValueType::FALSE) {}
  ObjValue(i8_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(u8_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(i16_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(u16_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(i32_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(u32_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(i64_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(u64_t n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(float n) noexcept : type_(ValueType::NUMERIC) { set_decimal(n); }
  ObjValue(double d) noexcept : type_(ValueType::NUMERIC) { as_.num = d; }
  ObjValue(BaseObject* o) noexcept : type_(ValueType::OBJECT) { as_.obj = o; }

  inline ValueType type() const noexcept { return type_; }
  inline ObjType objtype() const noexcept { return as_.obj->type(); }

  inline bool is_undefined() const noexcept { return type_ == ValueType::UNDEFINED; }
  inline bool is_nil() const noexcept { return type_ == ValueType::NIL; }
  inline bool is_boolean() const noexcept { return is_true() || is_false(); }
  inline bool is_numeric() const noexcept { return type_ == ValueType::NUMERIC; }
  inline bool is_object() const noexcept { return type_ == ValueType::OBJECT; }
  inline bool is_string() const noexcept { return is(ObjType::STRING); }
  inline bool is_list() const noexcept { return is(ObjType::LIST); }
  inline bool is_range() const noexcept { return is(ObjType::RANGE); }
  inline bool is_map() const noexcept { return is(ObjType::MAP); }
  inline bool is_module() const noexcept { return is(ObjType::MODULE); }
  inline bool is_function() const noexcept { return is(ObjType::FUNCTION); }
  inline bool is_foreign() const noexcept { return is(ObjType::FOREIGN); }
  inline bool is_upvalue() const noexcept { return is(ObjType::UPVALUE); }
  inline bool is_closure() const noexcept { return is(ObjType::CLOSURE); }
  inline bool is_fiber() const noexcept { return is(ObjType::FIBER); }
  inline bool is_class() const noexcept { return is(ObjType::CLASS); }
  inline bool is_instance() const noexcept { return is(ObjType::INSTANCE); }

  inline bool is_falsely() const noexcept { return is_nil() || is_false(); }
  inline bool operator==(const ObjValue& r) const noexcept { return is_equal(r); }
  inline bool operator!=(const ObjValue& r) const noexcept { return !is_equal(r); }

  inline bool as_boolean() const noexcept { return is_true(); }
  template <typename Int = int>
  inline Int as_integer() const noexcept { return Xt::as_type<Int>(as_.num); }
  inline double as_numeric() const noexcept { return as_.num; }
  inline BaseObject* as_object() const noexcept { return as_.obj; }

  StringObject* as_string() const noexcept { return as_.obj->as_string(); }
  const char* as_cstring() const noexcept { return as_.obj->as_cstring(); }
  ListObject* as_list() const noexcept { return as_.obj->as_list(); }
  RangeObject* as_range() const noexcept { return as_.obj->as_range(); }
  MapObject* as_map() const noexcept { return as_.obj->as_map(); }
  ModuleObject* as_module() const noexcept { return as_.obj->as_module(); }
  FunctionObject* as_function() const noexcept { return as_.obj->as_function(); }
  ForeignObject* as_foreign() const noexcept { return as_.obj->as_foreign(); }
  UpvalueObject* as_upvalue() const noexcept { return as_.obj->as_upvalue(); }
  ClosureObject* as_closure() const noexcept { return as_.obj->as_closure(); }
  FiberObject* as_fiber() const noexcept { return as_.obj->as_fiber(); }
  ClassObject* as_class() const noexcept { return as_.obj->as_class(); }
  InstanceObject* as_instance() const noexcept { return as_.obj->as_instance(); }

  bool is_same(const ObjValue& r) const noexcept;
  bool is_equal(const ObjValue& r) const noexcept;
  u32_t hasher() const noexcept;
  str_t stringify() const noexcept;
};

using Value = ObjValue;

inline std::ostream& operator<<(std::ostream& out, const Value& val) noexcept {
  return out << val.stringify();
}

// a heap-allocated string object
class StringObject final : public BaseObject {
  // number of bytes in the string, not including the null terminator
  sz_t size_{};

  // the hash value of the string's contents
  u32_t hash_{};

  // string's bytes followed by a null terminator
  char* data_{};

  StringObject(ClassObject* cls, char c) noexcept;
  StringObject(ClassObject* cls,
      const char* s, sz_t n, bool replace_owner = false) noexcept;
  virtual ~StringObject() noexcept;

  void hash_string() noexcept;
public:
  inline sz_t size() const noexcept { return size_; }
  inline sz_t length() const noexcept { return size_; }
  inline bool empty() const noexcept { return size_ == 0; }
  inline const char* cstr() const noexcept { return data_; }
  inline char* data() noexcept { return data_; }
  inline const char* data() const noexcept { return data_; }
  inline char& operator[](sz_t i) { return data_[i]; }
  inline const char& operator[](sz_t i) const { return data_[i]; }
  inline char& at(sz_t i) { return data_[i]; }
  inline const char& at(sz_t i) const { return data_[i]; }

  inline bool compare(StringObject* s) const {
    return this == s || (hash_ == s->hash_ && size_ == s->size_
        && std::memcmp(data_, s->data_, size_) == 0);
  }

  inline bool compare(const str_t& s) const {
    return size_ == s.size() && std::memcmp(data_, s.data(), size_) == 0;
  }

  inline bool compare(const char* s, sz_t n) const {
    return size_ == n && std::memcmp(data_, s, size_) == 0;
  }

  int find(StringObject* sub, sz_t off = 0) const;

  virtual bool is_equal(BaseObject* r) const override;
  virtual str_t stringify() const override;
  virtual u32_t hasher() const override;

  static StringObject* create(WrenVM& vm, char c);
  static StringObject* create(WrenVM& vm, const char* s, sz_t n);
  static StringObject* create(WrenVM& vm, const str_t& s);
  static StringObject* concat(WrenVM& vm, StringObject* s1, StringObject* s2);
  static StringObject* concat(WrenVM& vm, const char* s1, const char* s2);
  static StringObject* concat(WrenVM& vm, const str_t& s1, const str_t& s2);
  static StringObject* from_byte(WrenVM& vm, u8_t value);
  static StringObject* from_numeric(WrenVM& vm, double value);
  static StringObject* from_range(
      WrenVM& vm, StringObject* s, sz_t off, sz_t n, sz_t step);
  static StringObject* format(WrenVM& vm, const char* format, ...);
};

}
