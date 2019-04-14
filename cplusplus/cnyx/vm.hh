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

#include <list>
#include <optional>
#include <tuple>
#include <vector>
#include "common.hh"
#include "object.hh"

namespace nyx {

enum OpCode {
  OP_CONSTANT,
  OP_NIL, // nil value
  OP_POP, // pop operation
  OP_GET_LOCAL, // get local variable
  OP_SET_LOCAL, // set local variable
  OP_DEF_GLOBAL, // define global variable
  OP_GET_GLOBAL, // get global variable
  OP_SET_GLOBAL, // set global variable
  OP_GET_UPVALUE, // get upvalue
  OP_SET_UPVALUE, // set upvalue
  OP_GET_FIELD, // get field
  OP_SET_FIELD, // set field
  OP_EQ, // ==
  OP_NE, // !=
  OP_GT, // >
  OP_GE, // >=
  OP_LT, // <
  OP_LE, // <=
  OP_ADD, // +
  OP_SUB, // -
  OP_MUL, // *
  OP_DIV, // /
  OP_NOT, // !
  OP_NEG, // - negative
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP, // loop flow
  OP_CALL_0,
  OP_CALL_1,
  OP_CALL_2,
  OP_CALL_3,
  OP_CALL_4,
  OP_CALL_5,
  OP_CALL_6,
  OP_CALL_7,
  OP_CALL_8,
  OP_INVOKE_0,
  OP_INVOKE_1,
  OP_INVOKE_2,
  OP_INVOKE_3,
  OP_INVOKE_4,
  OP_INVOKE_5,
  OP_INVOKE_6,
  OP_INVOKE_7,
  OP_INVOKE_8,
  OP_CLOSURE,
  OP_CLOSE_UPVALUE,
  OP_RETURN, // return
  OP_CLASS, // class
  OP_SUBCLASS, // subclass
  OP_METHOD,
};

enum class InterpretResult {
  OK,
  COMPILE_ERROR,
  RUNTIME_ERROR,
};

class CallFrame;

class VM : private UnCopyable {
  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;

  table_t globals_;
  UpvalueObject* open_upvalues_{};

  std::list<BaseObject*> objects_;
  std::list<BaseObject*> gray_stack_;

  void define_native(const str_t& name, const NativeFunction& fn);
  void define_native(const str_t& name, NativeFunction&& fn);

  void create_class(StringObject* name, ClassObject* superclass = nullptr);
  void define_method(StringObject* name);

  void push(Value val);
  Value pop(void);
  Value peek(int distance = 0) const;
  void runtime_error(const char* format, ...);

  std::optional<bool> pop_boolean(void);
  std::optional<double> pop_numeric(void);
  std::optional<std::tuple<double, double>> pop_numerics(void);

  void collect(void);
  void print_stack(void);

  bool call_closure(ClosureObject* closure, int argc);
  bool call(Value callee, int argc = 0);
  bool invoke(Value receiver, StringObject* method, int argc);
  UpvalueObject* capture_upvalue(Value* local);
  void close_upvalues(Value* last);

  bool run(void);
public:
  VM(void);
  ~VM(void);

  inline void append_object(BaseObject* o) { objects_.push_back(o); }
  inline void invoke_push(Value v) { push(v); }
  inline Value invoke_pop(void) { return pop(); }

  void gray_value(Value v);
  void blacken_object(BaseObject* obj);
  void free_object(BaseObject* obj);

  InterpretResult interpret(const str_t& source_bytes);
};

}
