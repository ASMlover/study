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

class VM : private UnCopyable {
  struct CallFrame {
    FunctionObject* fun{};
    const u8_t* ip{};

    std::vector<Value> stack;
    CallFrame* caller{};

    CallFrame(void) {}
    CallFrame(FunctionObject* f, const u8_t* i, CallFrame* c = nullptr)
      : fun(f), ip(i), caller(c) {
    }

    void assign(FunctionObject* f, const u8_t* i, CallFrame* c = nullptr) {
      fun = f; ip = i; caller = c;
    }

    void append_to_stack(Value v) {
      stack.push_back(v);
    }

    Value pop_from_stack(void) {
      Value r = stack.back();
      stack.pop_back();
      return r;
    }

    Value peek_of_stack(int distance = 0) const {
      return stack[stack.size() - 1 - distance];
    }

    inline void set_stack_value(int i, Value v) { stack[i] = v; }
    inline Value get_stack_value(int i) const { return stack[i]; }
    inline Value get_fun_constant(int i) const { return fun->get_constant(i); }
  };

  CallFrame* frame_{};
  TableObject* globals_{};

  std::list<BaseObject*> objects_;
  std::list<BaseObject*> gray_stack_;

  inline void push(Value val) { return frame_->append_to_stack(val); }
  inline Value pop(void) { return frame_->pop_from_stack(); }
  inline Value peek(int distance = 0) const { return frame_->peek_of_stack(distance); }

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
