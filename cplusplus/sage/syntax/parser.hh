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
#include "../common/common.h"
#include "../lex/token_kinds.h"
#include "ast.hh"

namespace sage {

// expression     -> equality ;
// equality       -> comparison ( ( "is" | "!=" | "==" ) comparison )* ;
// comparison     -> addition ( ( ">" | ">=" | "<" | "<=" ) addition )* ;
// addition       -> multiplication ( ( "+" | "-" ) multiplication )* ;
// multiplication -> unary ( ( "*" | "/" | "%" ) unary )* ;
// unary          -> ( "not" | "!" | "-" ) unary | primary ;
// primary        -> INTEGER | DECIMAL | STRING | "true" | "false" | "nil"
//                | "(" expression ")" ;

class Token;
class ErrorReport;

class Parser : private UnCopyable {
  ErrorReport& err_report_;
  std::vector<Token> tokens_;
  std::size_t curpos_{};

  bool is_end(void) const;
  const Token& peek(void) const;
  const Token& prev(void) const;
  const Token& advance(void);
  bool check(TokenKind kind) const;
  bool match(const std::initializer_list<TokenKind>& kinds);
  const Token& consume(TokenKind kind, const std::string& message);
  const Token& consume(const std::initializer_list<TokenKind>& kinds,
      const std::string& message);

  ExprPtr expression(void);
  ExprPtr equality(void);
  ExprPtr comparison(void);
  ExprPtr addition(void);
  ExprPtr multiplication(void);
  ExprPtr unary(void);
  ExprPtr primary(void);
public:
  Parser(ErrorReport& err_report, const std::vector<Token>& tokens);

  ExprPtr parse(void);
};

}
