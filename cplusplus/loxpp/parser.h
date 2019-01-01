// Copyright (c) 2018 ASMlover. All rights reserved.
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

#include <exception>
#include <initializer_list>
#include <vector>
#include "token.h"
#include "expr.h"
#include "stmt.h"

class ParseError : public std::exception {
  virtual const char* what(void) const throw() override {
    return "Parser Error";
  }
};

class Parser {
  std::vector<Token> tokens_;
  int current_{0};

  bool is_end(void) const;
  Token peek(void) const;
  Token prev(void) const;
  Token advance(void);
  bool check(TokenType type) const;
  bool match(const std::initializer_list<TokenType>& types);
  Token consume(TokenType type, const std::string& msg);
  ParseError error(const Token& tok, const std::string& msg);
  void synchronize(void);

  ExprPtr expression(void);
  ExprPtr assignment(void);
  ExprPtr logical_or(void);
  ExprPtr logical_and(void);
  ExprPtr equality(void);
  ExprPtr comparison(void);
  ExprPtr addition(void);
  ExprPtr multiplication(void);
  ExprPtr unary(void);
  ExprPtr primary(void);

  StmtPtr declaration(void);
  StmtPtr var_declaration(void);
  StmtPtr statement(void);
  StmtPtr if_statement(void);
  StmtPtr print_statement(void);
  StmtPtr while_statement(void);
  std::vector<StmtPtr> block(void);
  StmtPtr expression_statement(void);
public:
  Parser(const std::vector<Token>& tokens);
  ExprPtr parse(void);

  std::vector<StmtPtr> parse_stmt(void);
};
