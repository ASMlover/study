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

#include <optional>
#include <tuple>
#include <vector>
#include <list>
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
  OP_RETURN, // return
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
};

enum class InterpretResult {
  OK,
  COMPILE_ERROR,
  RUNTIME_ERROR,
};

class CallFrame;

class VM : private UnCopyable {
  CallFrame* frame_{};
  TableObject* globals_{};

  std::list<BaseObject*> objects_;
  std::list<BaseObject*> gray_stack_;

  void push(Value val);
  Value pop(void);
  Value peek(int distance = 0) const;
  void runtime_error(const char* format, ...);

  std::optional<bool> pop_boolean(void);
  std::optional<double> pop_numeric(void);
  std::optional<std::tuple<double, double>> pop_numerics(void);

  void collect(void);
  void print_stack(void);

  void call(FunctionObject* fn);
  bool run(void);
public:
  VM(void);
  ~VM(void);

  inline void append_object(BaseObject* o) { objects_.push_back(o); }

  void gray_value(Value v);
  void blacken_object(BaseObject* obj);
  void free_object(BaseObject* obj);

  InterpretResult interpret(const str_t& source_bytes);
};

}
