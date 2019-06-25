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

#include "value.hh"

namespace wrencc {

enum class Code : u8_t {
  CONSTANT,     // load the constant at index [arg]
  NIL,          // push `nil` into the stack
  FALSE,        // push `false` into the stack
  TRUE,         // push `true` into the stack
  CLASS,        // define a new empty class and push it into stack
  SUBCLASS,     // pop a superclass from stack, push a new class that extends it

  // add a method for symbol [arg1] with body stored in constant [arg2] to
  // the class on the top of the stack, does not modify the stack
  METHOD_INSTANCE,

  // add a method for symbol [arg1] with body stored in constant [arg2] to
  // the metaclass of the class on the top of stack, does not modify the stack
  METHOD_STATIC,

  // add a constructor method for symbol [arg1] with body stored in constant
  // [arg2] to tthe metaclass of the class on the top of the stack. does not
  // modify the stack
  METHOD_CTOR,

  DUP,          // push a copy of the top of stack
  POP,          // pop and discard the top of stack
  LOAD_LOCAL,   // push the value in local slot [arg]
  STORE_LOCAL,  // store the top of the stack in local slot [arg], not pop it
  LOAD_GLOBAL,  // push the value in global slot [arg]
  STORE_GLOBAL, // store the top of the stack in global slot [arg], not pop it
  LOAD_FIELD,   // push the value of field in slot [arg] for current receiver
  STORE_FIELD,  // store the top of the stack in field slot [arg] in current receiver

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

  JUMP,         // jump the instruction pointer [arg] forward

  // jump the instruction pointer [arg] backward. pop and discard the
  // top of the stack
  LOOP,

  JUMP_IF,      // pop and if not truthy then jump the instruction pointer [arg] forward

  AND,          // if the top of the stack is false jump [arg], or pop and continue
  OR,           // if the top of the stack if non-false jump [arg], or pop and continue
  IS,           // pop [a] then [b] and push true if [b] is an instance of [a]

  END,
};

inline Code operator+(Code a, int b) {
  return Xt::as_type<Code>(Xt::as_type<int>(a) + b);
}

inline int operator-(Code a, Code b) {
  return Xt::as_type<int>(a) - Xt::as_type<int>(b);
}

class SymbolTable final : private UnCopyable {
  std::vector<str_t> symbols_;
public:
  inline int count(void) const { return Xt::as_type<int>(symbols_.size()); }
  inline const str_t& get_name(int i) const { return symbols_[i]; }

  int ensure(const str_t& name);
  int add(const str_t& name);
  int get(const str_t& name) const;
  void clear(void);
  void truncate(int count);
};

class VM final : private UnCopyable {
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
  std::vector<Value> globals_;

  Fiber* fiber_{};

  // how many bytes of object data have been allocated
  sz_t total_allocated_{};
  // the number of total allocated objects that will trigger the next GC
  sz_t next_gc_{1<<10}; // 1024

  std::vector<BaseObject*> objects_; // all currently allocated objects
  std::vector<BaseObject*> pinned_;

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
  void set_primitive(ClassObject* cls, const str_t& name, FiberPrimitiveFn fn);
  void set_global(ClassObject* cls, const str_t& name);
  const Value& get_global(const str_t& name) const;
  void pin_object(BaseObject* obj);
  void unpin_object(BaseObject* obj);

  void append_object(BaseObject* obj);
  void mark_object(BaseObject* obj);
  void mark_value(const Value& val);

  void interpret(const str_t& source_bytes);
  void call_function(Fiber& fiber, FunctionObject* fn, int argc);
};

}
