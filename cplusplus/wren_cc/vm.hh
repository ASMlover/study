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
  NO_VALUE,

  NIL,
  TRUE,
  FALSE,
  NUMERIC,
  OBJECT,
};

enum class ObjType {
  NIL,
  TRUE,
  FALSE,
  NUMERIC,
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
  ObjType type_{ObjType::NIL};
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
  ValueType type_{ValueType::NO_VALUE};
  BaseObject* obj_{};

  inline bool check(ObjType type) const { return is_object() && obj_->type() == type; }
public:
  Value(void) noexcept {}
  Value(nil_t) noexcept : type_(ValueType::NIL) {}
  Value(BaseObject* obj) noexcept;

  inline ValueType type(void) const { return type_; }
  inline ObjType objtype(void) const { return obj_->type(); }

  inline bool is_no_value(void) const { return type_ == ValueType::NO_VALUE; }
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

  double as_numeric(void) const;
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

class NilObject final : public BaseObject {
  NilObject(void) noexcept : BaseObject(ObjType::NIL) {}
public:
  virtual str_t stringify(void) const override;

  static NilObject* make_nil(VM& vm);
};

class BooleanObject final : public BaseObject {
  BooleanObject(bool b) noexcept
    : BaseObject(b ? ObjType::TRUE : ObjType::FALSE) {
  }
public:
  virtual str_t stringify(void) const override;

  static BooleanObject* make_boolean(VM& vm, bool b);
};

class NumericObject final : public BaseObject {
  double value_{};

  NumericObject(double d) noexcept : BaseObject(ObjType::NUMERIC), value_(d) {}
public:
  inline double value(void) const { return value_; }

  virtual str_t stringify(void) const override;

  static NumericObject* make_numeric(VM& vm, double d);
};

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
using PrimitiveFn = Value (*)(VM& vm, Fiber& fiber, Value* args);

enum class MethodType {
  NONE,
  PRIMITIVE,
  BLOCK,
};

struct Method {
  MethodType type{MethodType::NONE};
  union {
    PrimitiveFn primitive;
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
  inline void set_method(int i, MethodType type) { methods_[i].type = type; }
  inline void set_method(int i, PrimitiveFn fn) { methods_[i].primitive = fn; }
  inline void set_method(int i, FunctionObject* fn) { methods_[i].fn = fn; }
  inline void set_method(int i, MethodType type, PrimitiveFn fn) {
    methods_[i].type = type;
    methods_[i].primitive = fn;
  }
  inline void set_method(int i, MethodType type, FunctionObject* fn) {
    methods_[i].type = type;
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

enum class Code : u8_t {
  CONSTANT,     // load the constant at index [arg]
  NIL,          // push `nil` into the stack
  FALSE,        // push `false` into the stack
  TRUE,         // push `true` into the stack
  CLASS,        // define a new empty class and push it into stack
  SUBCLASS,     // pop a superclass from stack, push a new class that extends it

  // push the metaclass of the class on the top of the stack. does not
  // discard the class
  METACLASS,

  // method for symbol [arg1] with body stored in constant [arg2] to
  // the class on the top of stack, does not modify the stack
  METHOD,

  DUP,          // push a copy of the top of stack
  POP,          // pop and discard the top of stack
  LOAD_LOCAL,   // push the value in local slot [arg]
  STORE_LOCAL,  // store the top of the stack in local slot [arg], not pop it
  LOAD_GLOBAL,  // push the value in global slot [arg]
  STORE_GLOBAL, // store the top of the stack in global slot [arg], not pop it

  // invoke the method with symbol [arg], the number indicates the number of
  // arguments (not including the receiver)
  CALL_0,
  CALL_1,
  CALL_2,
  CALL_3,
  CALL_4,
  CALL_5,
  CALL_6,
  CALL_7,
  CALL_8,
  CALL_9,
  CALL_10,

  JUMP,         // jump the instruction pointer [arg1] forward
  JUMP_IF,      // pop and if not truthy then jump the instruction pointer [arg1] forward

  IS,           // pop [a] then [b] and push true if [b] is an instance of [a]

  END,
};

inline Code operator+(Code a, int b) {
  return Xt::as_type<Code>(Xt::as_type<int>(a) + b);
}

inline int operator-(Code a, Code b) {
  return Xt::as_type<int>(a) - Xt::as_type<int>(b);
}

class SymbolTable : private UnCopyable {
  std::vector<str_t> symbols_;
public:
  inline int count(void) const { return Xt::as_type<int>(symbols_.size()); }
  inline const str_t& get_name(int i) const { return symbols_[i]; }

  int ensure(const str_t& name);
  int add(const str_t& name);
  int get(const str_t& name) const;
  void clear(void);
};

class VM : private UnCopyable {
  static constexpr sz_t kMaxGlobals = 256;
  static constexpr sz_t kMaxPinned = 16;

  SymbolTable methods_;

  ClassObject* fn_class_{};
  ClassObject* bool_class_{};
  ClassObject* class_class_{};
  ClassObject* nil_class_{};
  ClassObject* num_class_{};
  ClassObject* obj_class_{};
  ClassObject* str_class_{};

  Value unsupported_{};

  SymbolTable global_symbols_;
  std::vector<Value> globals_{kMaxGlobals};

  Fiber* fiber_{};

  // how many bytes of object data have been allocated
  sz_t total_allocated_{};
  // the number of total allocated objects that will trigger the next GC
  sz_t next_gc_{1<<10}; // 1024

  std::vector<BaseObject*> objects_; // all currently allocated objects
  std::vector<Value> pinned_;

  Value interpret(FunctionObject* fn);

  ClassObject* get_class(const Value& val) const;

  void collect(void);
  void free_object(BaseObject* obj);
public:
  VM(void) noexcept;
  ~VM(void);

  inline void set_fn_cls(ClassObject* cls) { fn_class_ = cls; }
  inline void set_bool_cls(ClassObject* cls) { bool_class_ = cls; }
  inline void set_class_cls(ClassObject* cls) { class_class_ = cls; }
  inline void set_nil_cls(ClassObject* cls) { nil_class_ = cls; }
  inline void set_num_cls(ClassObject* cls) { num_class_ = cls; }
  inline void set_obj_cls(ClassObject* cls) { obj_class_ = cls; }
  inline void set_str_cls(ClassObject* cls) { str_class_ = cls; }

  inline ClassObject* fn_cls(void) const { return fn_class_; }
  inline ClassObject* bool_cls(void) const { return bool_class_; }
  inline ClassObject* class_cls(void) const { return class_class_; }
  inline ClassObject* nil_cls(void) const { return nil_class_; }
  inline ClassObject* num_cls(void) const { return num_class_; }
  inline ClassObject* obj_cls(void) const { return obj_class_; }
  inline ClassObject* str_cls(void) const { return str_class_; }

  inline void set_unsupported(const Value& unsupported) { unsupported_ = unsupported; }
  inline const Value& unsupported(void) const { return unsupported_; }

  inline SymbolTable& methods(void) { return methods_; }
  inline SymbolTable& gsymbols(void) { return global_symbols_; }
  void set_primitive(ClassObject* cls, const str_t& name, PrimitiveFn fn);
  void set_global(ClassObject* cls, const str_t& name);
  const Value& get_global(const str_t& name) const;
  void pin_object(const Value& value);
  void unpin_object(const Value& value);

  void append_object(BaseObject* obj);
  void mark_object(BaseObject* obj);
  void mark_value(const Value& val);

  void interpret(const str_t& source_bytes);
  void call_function(Fiber& fiber, FunctionObject* fn, int argc);
};

}
