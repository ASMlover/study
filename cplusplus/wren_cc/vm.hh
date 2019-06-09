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

enum class ObjType {
  NIL,
  TRUE,
  FALSE,
  NUMERIC,
  STRING,
  BLOCK,
  CLASS,
  INSTANCE,
};

enum class ObjFlag {
  MARKED = 0x01,
};

class BaseObject;
using Value = BaseObject*;

class NilObject;
class BooleanObject;
class NumericObject;
class StringObject;
class BlockObject;
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

  inline bool is_nil(void) const { return type_ == ObjType::NIL; }
  inline bool is_boolean(void) const { return type_ == ObjType::TRUE || type_ == ObjType::FALSE; }
  inline bool is_numeric(void) const { return type_ == ObjType::NUMERIC; }
  inline bool is_string(void) const { return type_ == ObjType::STRING; }
  inline bool is_block(void) const { return type_ == ObjType::BLOCK; }
  inline bool is_class(void) const { return type_ == ObjType::CLASS; }
  inline bool is_instance(void) const { return type_ == ObjType::INSTANCE; }

  bool as_boolean(void) const;
  double as_numeric(void) const;
  StringObject* as_string(void) const;
  const char* as_cstring(void) const;
  BlockObject* as_block(void) const;
  ClassObject* as_class(void) const;
  InstanceObject* as_instance(void) const;

  virtual str_t stringify(void) const = 0;
};

std::ostream& operator<<(std::ostream& out, Value val);

class NilObject final : public BaseObject {
  NilObject(void) noexcept : BaseObject(ObjType::NIL) {}
public:
  virtual str_t stringify(void) const override;

  static NilObject* make_nil(void);
};

class BooleanObject final : public BaseObject {
  BooleanObject(bool b) noexcept
    : BaseObject(b ? ObjType::TRUE : ObjType::FALSE) {
  }
public:
  virtual str_t stringify(void) const override;

  static BooleanObject* make_boolean(bool b);
};

class NumericObject final : public BaseObject {
  double value_{};

  NumericObject(double d) noexcept : BaseObject(ObjType::NUMERIC), value_(d) {}
public:
  inline double value(void) const { return value_; }

  virtual str_t stringify(void) const override;

  static NumericObject* make_numeric(double d);
};

class StringObject final : public BaseObject {
  int size_{};
  const char* value_{};

  StringObject(const char* s) noexcept
    : BaseObject(ObjType::STRING)
    , size_(Xt::as_type<int>(strlen(s)))
    , value_(s) {
  }
  virtual ~StringObject(void) {}
public:
  inline int size(void) const { return size_; }
  inline const char* cstr(void) const { return value_; }

  virtual str_t stringify(void) const override;

  static StringObject* make_string(const char* s);
};

class BlockObject final : public BaseObject {
  std::vector<u8_t> codes_;
  std::vector<Value> constants_;
  int num_locals_{};

  BlockObject(void) noexcept : BaseObject(ObjType::BLOCK) {}
public:
  inline const u8_t* codes(void) const { return codes_.data(); }
  inline const Value* constants(void) const { return constants_.data(); }
  inline int num_locals(void) const { return num_locals_; }
  inline int codes_count(void) const { return Xt::as_type<int>(codes_.size()); }
  inline int constants_count(void) const { return Xt::as_type<int>(constants_.size()); }
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline Value get_constant(int i) const { return constants_[i]; }
  inline void set_num_locals(int num_locals) { num_locals_ = num_locals; }

  template <typename T> inline int add_code(T c) {
    codes_.push_back(Xt::as_type<u8_t>(c));
    return Xt::as_type<int>(codes_.size()) - 1;
  }

  template <typename T> inline void set_code(int i, T c) {
    codes_[i] = Xt::as_type<u8_t>(c);
  }

  inline int add_constant(Value v) {
    constants_.push_back(v);
    return Xt::as_type<int>(constants_.size()) - 1;
  }

  virtual str_t stringify(void) const override;

  static BlockObject* make_block(void);
};

class VM;
class Fiber;
using PrimitiveFn = Value (*)(VM& vm, Fiber& fiber, int argc, Value* args);

enum class MethodType {
  NONE,
  PRIMITIVE,
  BLOCK,
};

struct Method {
  MethodType type{MethodType::NONE};
  union {
    PrimitiveFn primitive;
    BlockObject* block;
  };
};

class ClassObject final : public BaseObject {
  static constexpr sz_t kMaxMethods = 256;

  ClassObject* meta_class_{};
  std::vector<Method> methods_{kMaxMethods};

  ClassObject(void) noexcept;
  ClassObject(ClassObject* meta_class) noexcept;
public:
  inline ClassObject* meta_class(void) const { return meta_class_; }
  inline int methods_count(void) const { return Xt::as_type<int>(methods_.size()); }
  inline Method& get_method(int i) { return methods_[i]; }
  inline void set_method(int i, MethodType type) { methods_[i].type = type; }
  inline void set_method(int i, PrimitiveFn fn) { methods_[i].primitive = fn; }
  inline void set_method(int i, BlockObject* b) { methods_[i].block = b; }
  inline void set_method(int i, MethodType type, PrimitiveFn fn) {
    methods_[i].type = type;
    methods_[i].primitive = fn;
  }
  inline void set_method(int i, MethodType type, BlockObject* block) {
    methods_[i].type = type;
    methods_[i].block = block;
  }

  virtual str_t stringify(void) const override;

  static ClassObject* make_class(void);
};

class InstanceObject final : public BaseObject {
  ClassObject* cls_{};
  // TODO: need add instance fields

  InstanceObject(ClassObject* cls) noexcept;
public:
  inline ClassObject* cls(void) const { return cls_; }

  virtual str_t stringify(void) const override;

  static InstanceObject* make_instance(ClassObject* cls);
};

enum class Code : u8_t {
  CONSTANT, // load the constant at index [arg]
  NIL, // push `nil` into the stack
  FALSE, // push `false` into the stack
  TRUE, // push `true` into the stack
  CLASS, // define a new empty class and push it into stack
  METHOD, // method for symbol [arg1] with body stored in constant [arg2] to
          // the class on the top of stack, does not modify the stack
  DUP, // push a copy of the top of stack
  POP, // pop and discard the top of stack
  LOAD_LOCAL, // push the value in local slot [arg]
  STORE_LOCAL, // store the top of the stack in local slot [arg], not pop it
  LOAD_GLOBAL, // push the value in global slot [arg]
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

  JUMP, // jump the instruction pointer [arg1] forward
  JUMP_IF, // pop and if not truthy then jump the instruction pointer [arg1] forward

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

  SymbolTable symbols_;

  ClassObject* block_class_{};
  ClassObject* bool_class_{};
  ClassObject* class_class_{};
  ClassObject* nil_class_{};
  ClassObject* num_class_{};
  ClassObject* str_class_{};

  Value unsupported_{};

  SymbolTable global_symbols_;
  std::vector<Value> globals_{kMaxGlobals};

  Value interpret(BlockObject* block);
public:
  VM(void);

  inline void set_block_cls(ClassObject* cls) { block_class_ = cls; }
  inline void set_bool_cls(ClassObject* cls) { bool_class_ = cls; }
  inline void set_class_cls(ClassObject* cls) { class_class_ = cls; }
  inline void set_nil_cls(ClassObject* cls) { nil_class_ = cls; }
  inline void set_num_cls(ClassObject* cls) { num_class_ = cls; }
  inline void set_str_cls(ClassObject* cls) { str_class_ = cls; }

  inline ClassObject* block_cls(void) const { return block_class_; }
  inline ClassObject* bool_cls(void) const { return bool_class_; }
  inline ClassObject* class_cls(void) const { return class_class_; }
  inline ClassObject* nil_cls(void) const { return nil_class_; }
  inline ClassObject* num_cls(void) const { return num_class_; }
  inline ClassObject* str_cls(void) const { return str_class_; }

  inline void set_unsupported(Value unsupported) { unsupported_ = unsupported; }
  inline Value unsupported(void) const { return unsupported_; }

  inline SymbolTable& symbols(void) { return symbols_; }
  inline SymbolTable& gsymbols(void) { return global_symbols_; }
  void set_primitive(ClassObject* cls, const str_t& name, PrimitiveFn fn);
  void set_global(ClassObject* cls, const str_t& name);

  void interpret(const str_t& source_bytes);
  void call_block(Fiber& fiber, BlockObject* block, int argc);
};

}
