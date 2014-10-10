// Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __SNIPPET_HEADER_H__
#define __SNIPPET_HEADER_H__

#include "global.h"

enum class OpCode {
  OPCODE_INVAL = 0,
  OPCODE_EXIT,  // exit vm
  OPCODE_JMP,   // jump to instruction
  OPCODE_BOOL,  // boolean value
  OPCODE_INT,   // integer value
  OPCODE_REAL,  // float/double value
  OPCODE_STRING,// string value
  OPCODE_UNDEF, // undefined value = nil value
  OPCODE_VAR,   // variables
  OPCODE_ASSIGN,// assignment (=)
  OPCODE_DEL,   // delete vairable
  OPCODE_APPEND,// append element(array)
  OPCODE_COND,  // condition (for if, while, for and switch)
  OPCODE_DO,    // condition (for do ... while)
  OPCODE_PRINT, // console output (print)
  OPCODE_READ,  // read from console
  OPCODE_AND,   // and (&&) condition
  OPCODE_OR,    // or (||) condition
  OPCODE_LT,    // <
  OPCODE_GT,    // >
  OPCODE_LE,    // <=
  OPCODE_GE,    // >=
  OPCODE_EQ,    // ==
  OPCODE_NE,    // != (unequal)
  OPCODE_NOT,   // ! (not condition)
  OPCODE_ADD,   // +
  OPCODE_SUB,   // -
  OPCODE_MUL,   // *
  OPCODE_DIV,   // /
  OPCODE_MOD,   // %
  OPCODE_POW,   // ^
};

class Token;
class Expr;
class Snippet : private UnCopyable {
  typedef std::shared_ptr<Expr> ExprPtr;

  std::set<char>        delimiter_;
  std::set<std::string> keywords_;

  std::string         code_;
  std::vector<Token>  tokens_;
  ExprPtr             syntax_tree_;
public:
  Snippet(void);
  ~Snippet(void);

  bool Load(const std::string& path);
  bool Compile(const std::string& path);
  void Run(const std::string& path);
};

#endif  // __SNIPPET_HEADER_H__
