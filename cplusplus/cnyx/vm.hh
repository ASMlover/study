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

#include <vector>
#include <list>
#include "common.hh"
#include "object.hh"

namespace nyx {

enum OpCode {
  OP_CONSTANT,
  OP_POP, // pop operation
  OP_DEF_GLOBAL, // define global variable
  OP_GET_GLOBAL, // get global variable
  OP_SET_GLOBAL, // set global variable
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
};

class VM : private UnCopyable {
  TableObject* globals_{};
  std::vector<Value> stack_;
  std::list<Object*> objects_;
  std::list<Object*> gray_stack_;

  inline void push(Value val) {
    stack_.push_back(val);
  }

  inline Value pop(void) {
    Value val = stack_.back();
    stack_.pop_back();
    return val;
  }

  inline Value peek(int distance = 0) const {
    return stack_[stack_.size() - 1 - distance];
  }

  bool pop_boolean(bool& v);
  bool pop_numeric(double& v);
  bool pop_numerics(double& a, double& b);

  void collect(void);
  void print_stack(void);
  void run(FunctionObject* fn);
public:
  VM(void);
  ~VM(void);

  void put_in(Object* o);

  void gray_value(Value v);
  void blacken_object(Object* obj);
  void free_object(Object* obj);

  void interpret(const std::string& source_bytes);
};

}
