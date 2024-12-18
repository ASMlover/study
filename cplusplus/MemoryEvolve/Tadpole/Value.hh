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

#include <functional>
#include <Core/MEvolve.hh>

namespace _mevo::tadpole {

enum class ObjType {
  STRING,
  NATIVE,
  FUNCTION,
  UPVALUE,
  CLOSURE,
};

class VM;
class Chunk;

class StringObject;
class NativeObject;
class FunctionObject;
class UpvalueObject;
class ClosureObject;

class BaseObject : private UnCopyable {
  ObjType type_;
  bool marked_{};
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject() {}

  inline ObjType type() const noexcept { return type_; }
  inline bool marked() const noexcept { return marked_; }
  inline void set_marked(bool marked = true) noexcept { marked_ = marked; }

  virtual str_t stringify() const { return "<object>"; }
  virtual bool is_truthy() const { return true; }
  virtual void gc_blacken(VM& vm) {}

  StringObject* as_string();
  const char* as_cstring();
  NativeObject* as_native();
  FunctionObject* as_function();
  UpvalueObject* as_upvalue();
  ClosureObject* as_closure();
};

enum class ValueType {
  NIL,
  BOOLEAN,
  NUMERIC,
  OBJECT,
};

class Value final : public Copyable {
  ValueType type_{ValueType::NIL};
  union {
    bool boolean;
    double numeric;
    BaseObject* object{};
  } as_;

  inline bool _is(ObjType type) const noexcept {
    return is_object() && objtype() == type;
  }

  template <typename T> inline void set_numeric(T x) noexcept {
    as_.numeric = as_type<double>(x);
  }
public:
  Value() noexcept {}
  Value(nil_t) noexcept {}
  Value(bool b) noexcept : type_(ValueType::BOOLEAN) { as_.boolean = b; }
  Value(i8_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u8_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i16_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u16_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i32_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u32_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(i64_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(u64_t n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(float n) noexcept : type_(ValueType::NUMERIC) { set_numeric(n); }
  Value(double d) noexcept : type_(ValueType::NUMERIC) { as_.numeric = d; }
  Value(BaseObject* o) noexcept : type_(ValueType::OBJECT) { as_.object = o; }

  inline ObjType objtype() const noexcept { return as_.object->type(); }

  inline bool is_nil() const noexcept { return type_ == ValueType::NIL; }
  inline bool is_boolean() const noexcept { return type_ == ValueType::BOOLEAN; }
  inline bool is_numeric() const noexcept { return type_ == ValueType::NUMERIC; }
  inline bool is_object() const noexcept { return type_ == ValueType::OBJECT; }
  inline bool is_string() const noexcept { return _is(ObjType::STRING); }
  inline bool is_native() const noexcept { return _is(ObjType::NATIVE); }
  inline bool is_function() const noexcept { return _is(ObjType::FUNCTION); }
  inline bool is_closure() const noexcept { return _is(ObjType::CLOSURE); }
  inline bool is_upvalue() const noexcept { return _is(ObjType::UPVALUE); }

  inline bool as_boolean() const noexcept { return as_.boolean; }
  inline double as_numeric() const noexcept { return as_.numeric; }
  inline BaseObject* as_object() const noexcept { return as_.object; }
  inline StringObject* as_string() const noexcept { return as_.object->as_string(); }
  inline const char* as_cstring() const noexcept { return as_.object->as_cstring(); }
  inline NativeObject* as_native() const noexcept { return as_.object->as_native(); }
  inline FunctionObject* as_function() const noexcept { return as_.object->as_function(); }
  inline ClosureObject* as_closure() const noexcept { return as_.object->as_closure(); }
  inline UpvalueObject* as_upvalue() const noexcept { return as_.object->as_upvalue(); }

  bool is_truthy() const;
  str_t stringify() const;
};

inline std::ostream& operator<<(std::ostream& out, Value val) {
  return out << val.stringify();
}

using NativeFn = std::function<Value (int, Value*)>;

class StringObject final : public BaseObject {
  char* data_{};
  sz_t size_{};
  u32_t hash_{};
public:
  StringObject(const char* s, sz_t n, u32_t h, bool replace_owner = false) noexcept;
  virtual ~StringObject();

  inline const char* data() const noexcept { return data_; }
  inline const char* cstr() const noexcept { return data_; }
  inline sz_t size() const noexcept { return size_; }
  inline u32_t hash() const noexcept { return hash_; }

  virtual str_t stringify() const override;

  static StringObject* create(VM& vm, const char* s, sz_t n);
  static StringObject* concat(VM& vm, StringObject* s1, StringObject* s2);

  template <typename T>
  static StringObject* create(VM& vm, const char* s, T n) {
    return create(vm, s, as_type<sz_t>(n));
  }

  static StringObject* create(VM& vm, const str_t& s) {
    return create(vm, s.data(), s.size());
  }
};

class NativeObject final : public BaseObject {
  NativeFn fn_{};
public:
  NativeObject(NativeFn&& fn) noexcept
    : BaseObject(ObjType::NATIVE), fn_(fn) {
  }

  inline NativeFn fn() const noexcept { return fn_; }

  virtual str_t stringify() const override;

  static NativeObject* create(VM& vm, NativeFn&& fn);
};

class FunctionObject final : public BaseObject {
  StringObject* name_{};
  int arity_{};
  int upvalues_count_{};
  Chunk* chunk_{};
public:
  FunctionObject(StringObject* name = nullptr) noexcept;
  virtual ~FunctionObject();

  inline StringObject* name() const noexcept { return name_; }
  inline const char* name_asstr() const noexcept { return name_ ? name_->cstr() : "<tadpole>"; }
  inline void set_name(StringObject* name) noexcept { name_ = name; }
  inline int arity() const noexcept { return arity_; }
  inline int inc_arity() noexcept { return arity_++; }
  inline int upvalues_count() const noexcept { return upvalues_count_; }
  inline int inc_upvalues_count() noexcept { return upvalues_count_++; }
  inline Chunk* chunk() const noexcept { return chunk_; }

  virtual str_t stringify() const override;
  virtual void gc_blacken(VM& vm) override;

  static FunctionObject* create(VM& vm, StringObject* name = nullptr);
};

class UpvalueObject final : public BaseObject {
  Value* value_{};
  Value closed_{};
  UpvalueObject* next_{};
public:
  UpvalueObject(Value* value, UpvalueObject* next = nullptr) noexcept;

  inline Value* value() const noexcept { return value_; }
  inline Value* value_asptr() const noexcept { return value_; }
  inline const Value& value_asref() const noexcept { return *value_; }
  inline void set_value(Value* value) noexcept { value_ = value; }
  inline void set_value(const Value& value) noexcept { *value_ = value; }
  inline const Value& closed() const noexcept { return closed_; }
  inline Value* closed_asptr() noexcept { return &closed_; }
  inline const Value& closed_asref() const noexcept { return closed_; }
  inline void set_closed(const Value& closed) noexcept { closed_ = closed; }
  inline void set_closed(Value* closed) noexcept { closed_ = *closed; }
  inline UpvalueObject* next() const noexcept { return next_; }
  inline void set_next(UpvalueObject* next) noexcept { next_ = next; }

  virtual str_t stringify() const override;
  virtual void gc_blacken(VM& vm) override;

  static UpvalueObject* create(VM& vm, Value* value, UpvalueObject* next = nullptr);
};

class ClosureObject final : public BaseObject {
  FunctionObject* fn_{};
  int upvalues_count_{};
  UpvalueObject** upvalues_{};
public:
  ClosureObject(FunctionObject* fn) noexcept;
  virtual ~ClosureObject();

  inline FunctionObject* fn() const noexcept { return fn_; }
  inline int upvalues_count() const noexcept { return upvalues_count_; }
  inline UpvalueObject* get_upvalue(int i) const noexcept { return upvalues_[i]; }
  inline void set_upvalue(int i, UpvalueObject* u) noexcept { upvalues_[i] = u; }

  virtual str_t stringify() const override;
  virtual void gc_blacken(VM& vm) override;

  static ClosureObject* create(VM& vm, FunctionObject* fn);
};

}
