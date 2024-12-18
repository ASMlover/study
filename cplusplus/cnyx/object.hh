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
#include <string>
#include "common.hh"
#include "value.hh"
#include "chunk.hh"

namespace nyx {

class Value;
using NativeFunction = std::function<Value (int argc, Value* args)>;

enum class ObjType {
  STRING,
  CLOSURE,
  FUNCTION,
  NATIVE,
  UPVALUE,
  CLASS,
  INSTANCE,
  BOUND_METHOD,
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
  str_t type_name(void) const;

  virtual sz_t size_bytes(void) const = 0;
  virtual str_t stringify(void) const = 0;
  virtual bool is_equal(BaseObject* other) const = 0;
  virtual void blacken(VM& vm) {}
};

inline ObjType obj_type(const Value& v) { return v.as_object()->type(); }
inline bool is_obj_type(const Value& v, ObjType type) {
  return v.is_object() && v.as_object()->type() == type;
}

std::ostream& operator<<(std::ostream& out, BaseObject* obj);

void gray_table(VM& vm, table_t& tbl);
void remove_table_undark(table_t& tbl);

class StringObject final : public BaseObject {
  int count_{};
  u32_t hash_{};
  char* chars_{};

  StringObject(const char* s, int n, u32_t hash, bool copy = true);
  virtual ~StringObject(void);
public:
  inline int count(void) const { return count_; }
  inline char* chars(void) { return chars_; }
  inline const char* chars(void) const { return chars_; }
  inline char get_element(int i) const { return chars_[i]; }
  inline void set_element(int i, char c) { chars_[i] = c; }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;

  static StringObject* create(VM& vm, const str_t& s);
  static StringObject* create(VM& vm, const char* s, int n);
  static StringObject* concat(VM& vm, StringObject* a, StringObject* b);
};

class FunctionObject final : public BaseObject {
  Chunk chunk_{};

  int arity_{};
  int upvalues_count_{};

  // debug information
  StringObject* name_{};

  FunctionObject(void);
  virtual ~FunctionObject(void);
public:
  inline int codes_count(void) const { return chunk_.codes_count(); }
  inline u8_t* codes(void) { return chunk_.codes(); }
  inline const u8_t* codes(void) const { return chunk_.codes(); }
  inline void set_code(int i, u8_t c) { chunk_.set_code(i, c); }
  inline u8_t get_code(int i) const { return chunk_.get_code(i); }
  inline int get_codeline(int i) const { return chunk_.get_line(i); }
  inline int arity(void) const { return arity_; }
  inline void inc_arity(void) { ++arity_; }
  inline int upvalues_count(void) const { return upvalues_count_; }
  inline int inc_upvalues_count(void) { return upvalues_count_++; }
  inline int constants_count(void) const { return chunk_.constants_count(); }
  inline Value* constants(void) { return chunk_.constants(); }
  inline const Value* constants(void) const { return chunk_.constants(); }
  inline Value& get_constant(int i) { return chunk_.get_constant(i); }
  inline const Value& get_constant(int i) const { return chunk_.get_constant(i); }
  inline StringObject* name(void) const { return name_; }
  inline void set_name(StringObject* name) { name_ = name; }
  inline int append_code(u8_t c, int lineno = 0) { return chunk_.write(c, lineno); }
  inline int append_constant(const Value& v) { return chunk_.add_constant(v); }

  inline void disassemble(const str_t& name) { chunk_.disassemble(name); }
  inline int disassemble_instruction(int i) {
    return chunk_.disassemble_instruction(i);
  }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static FunctionObject* create(VM& vm);
};

class NativeObject final : public BaseObject {
  NativeFunction fn_{};

  NativeObject(const NativeFunction& fn)
    : BaseObject(ObjType::NATIVE), fn_(fn) {}
  NativeObject(NativeFunction&& fn)
    : BaseObject(ObjType::NATIVE), fn_(std::move(fn)) {}
  virtual ~NativeObject(void) {}
public:
  inline void set_function(const NativeFunction& fn) { fn_ = fn; }
  inline void set_function(NativeFunction&& fn) { fn_ = std::move(fn); }
  inline NativeFunction get_function(void) const { return fn_; }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;

  static NativeObject* create(VM& vm, const NativeFunction& fn);
  static NativeObject* create(VM& vm, NativeFunction&& fn);
};

class UpvalueObject final : public BaseObject {
  Value* value_{};
  Value closed_{};
  UpvalueObject* next_{};

  UpvalueObject(Value* slot) : BaseObject(ObjType::UPVALUE) , value_(slot) {}
  virtual ~UpvalueObject(void) {}
public:
  inline Value* value(void) const { return value_; }
  inline Value& closed(void) { return closed_; }
  inline const Value& closed(void) const { return closed_; }
  inline UpvalueObject* next(void) const { return next_; }
  inline void set_value(Value* value) { value_ = value; }
  inline void set_closed(const Value& closed) { closed_ = closed; }
  inline void set_next(UpvalueObject* next) { next_ = next; }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static UpvalueObject* create(VM& vm, Value* slot);
};

class ClosureObject final : public BaseObject {
  FunctionObject* function_{};
  UpvalueObject** upvalues_{};
  int upvalues_count_{};

  ClosureObject(FunctionObject* fn);
  virtual ~ClosureObject(void);
public:
  inline FunctionObject* get_function(void) const { return function_; }
  inline UpvalueObject** upvalues(void) const { return upvalues_; }
  inline int upvaules_count(void) const { return upvalues_count_; }
  inline UpvalueObject* get_upvalue(int i) const { return upvalues_[i]; }
  inline void set_upvalue(int i, UpvalueObject* upvalue) { upvalues_[i] = upvalue; }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static ClosureObject* create(VM& vm, FunctionObject* fn);
};

class ClassObject final : public BaseObject {
  StringObject* name_{};
  ClassObject* superclass_{};
  table_t methods_;

  ClassObject(StringObject* name, ClassObject* superclass);
  virtual ~ClassObject(void);
public:
  inline StringObject* name(void) const { return name_; }
  inline ClassObject* superclass(void) const { return superclass_; }

  inline void bind_method(StringObject* name, const Value& method) {
    methods_[name->chars()] = method;
  }

  inline std::optional<Value> get_method(StringObject* name) const {
    return get_method(name->chars());
  }

  inline std::optional<Value> get_method(const str_t& name) const {
    if (auto it = methods_.find(name); it != methods_.end())
      return {it->second};
    return {};
  }

  void inherit_from(ClassObject* superclass);

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static ClassObject* create(VM& vm, StringObject* name, ClassObject* superclass);
};

class InstanceObject final : public BaseObject {
  ClassObject* class_{};
  table_t fields_;

  InstanceObject(ClassObject* klass);
  virtual ~InstanceObject(void);
public:
  inline ClassObject* get_class(void) const { return class_; }

  inline void set_field(StringObject* name, const Value& value) {
    fields_[name->chars()] = value;
  }

  inline std::optional<Value> get_field(StringObject* name) const {
    return get_field(name->chars());
  }

  inline std::optional<Value> get_field(const str_t& name) const {
    if (auto it = fields_.find(name); it != fields_.end())
      return {it->second};
    return {};
  }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static InstanceObject* create(VM& vm, ClassObject* klass);
};

class BoundMethodObject final : public BaseObject {
  Value receiver_{};
  ClosureObject* method_{};

  BoundMethodObject(const Value& receiver, ClosureObject* method);
  virtual ~BoundMethodObject(void);
public:
  inline Value& receiver(void) { return receiver_; }
  inline const Value& receiver(void) const { return receiver_; }
  inline ClosureObject* method(void) const { return method_; }

  virtual sz_t size_bytes(void) const override;
  virtual str_t stringify(void) const override;
  virtual bool is_equal(BaseObject* other) const override;
  virtual void blacken(VM& vm) override;

  static BoundMethodObject* create(
      VM& vm, const Value& receiver, ClosureObject* method);
};

}
