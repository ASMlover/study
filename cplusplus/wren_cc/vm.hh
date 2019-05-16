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
  NUMERIC,
  BLOCK,
  CLASS,
};

enum class ObjFlag {
  MARKED = 0x01,
};

class BaseObject;
using Value = BaseObject*;

class BaseObject : private UnCopyable {
  ObjType type_{};
  ObjFlag flag_{};
public:
  BaseObject(ObjType type) noexcept : type_(type) {}
  virtual ~BaseObject(void) {}

  inline ObjType type(void) const { return type_; }
  inline ObjFlag flag(void) const { return flag_; }

  virtual str_t stringify(void) const = 0;
};

std::ostream& operator<<(std::ostream& out, Value val);

class NumericObject : public BaseObject {
  double value_{};

  NumericObject(double d) noexcept : BaseObject(ObjType::NUMERIC), value_(d) {}
  virtual ~NumericObject(void) {}
public:
  inline double value(void) const { return value_; }

  virtual str_t stringify(void) const override;

  static NumericObject* make_numeric(double d);
};

class BlockObject : public BaseObject {
  std::vector<u8_t> codes_;
  std::vector<Value> constants_;
  int num_locals_{};

  BlockObject(void) noexcept : BaseObject(ObjType::BLOCK) {}
  virtual ~BlockObject(void) {}
public:
  inline const u8_t* codes(void) const { return codes_.data(); }
  inline const Value* constants(void) const { return constants_.data(); }
  inline int num_locals(void) const { return num_locals_; }
  inline int codes_count(void) const { return Xt::as_type<int>(codes_.size()); }
  inline int constants_count(void) const { return Xt::as_type<int>(constants_.size()); }
  inline u8_t get_code(int i) const { return codes_[i]; }
  inline Value get_constant(int i) const { return constants_[i]; }
  inline void set_num_locals(int num_locals) { num_locals_ = num_locals; }

  template <typename T> inline void add_code(T c) {
    codes_.push_back(Xt::as_type<u8_t>(c));
  }

  inline u8_t add_constant(Value v) {
    constants_.push_back(v);
    return Xt::as_type<int>(constants_.size() - 1);
  }

  virtual str_t stringify(void) const override;

  static BlockObject* make_block(void);
};

using PrimitiveFn = Value (*)(Value);

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

class ClassObject : public BaseObject {
  static constexpr sz_t kMaxMethods = 256;

  std::vector<Method> methods_{kMaxMethods};

  ClassObject(void) noexcept;
  virtual ~ClassObject(void) {}
public:
  inline int methods_count(void) const { return Xt::as_type<int>(methods_.size()); }
  inline Method& get_method(int i) { return methods_[i]; }
  inline void set_method(int i, MethodType type, PrimitiveFn fn) {
    methods_[i].type = type;
    methods_[i].primitive = fn;
  }

  virtual str_t stringify(void) const override;

  static ClassObject* make_class(void);
};

enum class Code : u8_t {
  CONSTANT, // load the constant at index [arg]
  CLASS, // define a new empty class and push it into stack
  DUP, // push a copy of the top of stack
  POP, // pop and discard the top of stack
  LOAD_LOCAL, // push the value in local slot [arg]
  STORE_LOCAL, // store the top of the stack in local slot [arg], not pop it
  CALL, // invoke the method with symbol [arg]

  END,
};

class SymbolTable : private UnCopyable {
  std::vector<str_t> symbols_;
public:
  inline int count(void) const { return Xt::as_type<int>(symbols_.size()); }

  int ensure(const str_t& name);
  int add(const str_t& name);
  int get(const str_t& name) const;
  void clear(void);
};

class VM : private UnCopyable {
  SymbolTable symbols_;
  ClassObject* num_class_;

  Value interpret(BlockObject* block);
public:
  VM(void);

  inline SymbolTable& symbols(void) { return symbols_; }

  void interpret(const str_t& source_bytes);
};

}
