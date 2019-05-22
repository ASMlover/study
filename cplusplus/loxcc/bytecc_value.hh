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
#include <optional>
#include <ostream>
#include "common.hh"

namespace loxcc::bytecc {

enum class ObjType {
  STRING,
  NATIVE,
  FUNCTION,
  UPVALUE,
  CLOSURE,
  CLASS,
  INSTANCE,
  BOUND_METHOD,
};

class VM;

class BaseObject : private UnCopyable {
  ObjType type_{};
  bool marked_{};
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject(void) {}

  inline ObjType type(void) const { return type_; }
  inline bool marked(void) const { return marked_; }
  inline void set_marked(bool marked = true) { marked_ = marked; }

  virtual sz_t size_bytes(void) const = 0;
  virtual str_t stringify(void) const = 0;
  virtual bool is_truthy(void) const { return true; }
  virtual void blacken(VM& vm) {}
};

class StringObject;
class NativeObject;
class FunctionObject;
class UpvalueObject;
class ClosureObject;
class ClassObject;
class InstanceObject;
class BoundMehtodObject;

enum class ValueType {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT,
};

class Value final : public Copyable {
  using NativeTp = std::function<Value (int argc, Value* args)>;

  ValueType type_{ValueType::NIL};
  union {
    bool boolean;
    double numeric;
    BaseObject* object;
  } as_{};

  template <typename T> inline void set_numeric(T x) {
    as_.numeric = Xt::as_type<double>(x);
  }

  inline bool check(ObjType type) const {
    return is_object() && as_.object->type() == type;
  }
public:
  Value(void) noexcept {}
  Value(nil_t) noexcept {}
  Value(bool b) noexcept : type_(ValueType::BOOLEAN) { as_.boolean = b; }
  Value(i8_t x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(u8_t x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(i16_t x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(u16_t x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(i32_t x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(u32_t x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(i64_t x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(u64_t x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(float x) noexcept : type_(ValueType::NUMERIC) { set_numeric(x); }
  Value(double d) noexcept : type_(ValueType::NUMERIC) { as_.numeric = d; }
  Value(BaseObject* o) noexcept : type_(ValueType::OBJECT) { as_.object = o; }

  Value(const Value& r) noexcept
    : type_(r.type_) {
    if (type_ == ValueType::OBJECT)
      as_.object = r.as_.object;
    else
      as_.numeric = r.as_.numeric;
  }

  Value(Value&& r) noexcept
    : type_(std::move(r.type_)) {
    if (type_ == ValueType::OBJECT)
      as_.object = std::move(r.as_.object);
    else
      as_.numeric = std::move(r.as_.numeric);
  }

  inline Value& operator=(const Value& r) noexcept {
    if (this != &r) {
      type_ = r.type_;
      if (type_ == ValueType::OBJECT)
        as_.object = r.as_.object;
      else
        as_.numeric = r.as_.numeric;
    }
    return *this;
  }

  inline Value& operator=(Value&& r) noexcept {
    if (this != &r) {
      type_ = std::move(r.type_);
      if (type_ == ValueType::OBJECT)
        as_.object = std::move(r.as_.object);
      else
        as_.numeric = std::move(r.as_.numeric);
    }
    return *this;
  }

  inline bool is_nil(void) const { return type_ == ValueType::NIL; }
  inline bool is_boolean(void) const { return type_ == ValueType::BOOLEAN; }
  inline bool is_numeric(void) const { return type_ == ValueType::NUMERIC; }
  inline bool is_object(void) const { return type_ == ValueType::OBJECT; }
  inline bool is_string(void) const { return check(ObjType::STRING); }
  inline bool is_native(void) const { return check(ObjType::NATIVE); }
  inline bool is_function(void) const { return check(ObjType::FUNCTION); }
  inline bool is_upvalue(void) const { return check(ObjType::UPVALUE); }
  inline bool is_closure(void) const { return check(ObjType::CLOSURE); }
  inline bool is_class(void) const { return check(ObjType::CLASS); }
  inline bool is_instance(void) const { return check(ObjType::INSTANCE); }
  inline bool is_bound_method(void) const { return check(ObjType::BOUND_METHOD); }
};

}
