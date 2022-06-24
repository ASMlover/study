// Copyright (c) 2022 ASMlover. All rights reserved.
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
#include "common.hh"
#include "token.hh"
#include "errors.hh"
#include "expr.hh"

namespace loxpp::parser {

// expression -> equality ;
// equality   -> comparison ( ( "!=" | "==" ) comparison )* ;
// comparison -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
// term       -> factor ( ( "-" | "+" ) factor )* ;
// factor     -> unary ( ( "/" | "*" ) unary )* ;
// unary      -> ( "!" | "-" ) unary | primary ;
// primary    -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;

class Parser final : private UnCopyable {
  std::vector<Token> tokens_;
  sz_t current_{};

  inline const Token& peek() const noexcept { return tokens_[current_]; }
  inline const Token& prev() const noexcept { return tokens_[current_ - 1]; }
  inline bool is_at_end() const noexcept { return peek().type() == TokenType::TK_EOF; }
  inline bool check(TokenType type) const noexcept { return is_at_end() ? false : peek().type() == type; }

  Token advance() noexcept {
    if (!is_at_end())
      ++current_;
    return prev();
  }

  bool match(const std::initializer_list<TokenType>& types) noexcept {
    for (auto type : types) {
      if (check(type)) {
        advance();
        return true;
      }
    }
    return false;
  }

  Token consume(TokenType type, const str_t& message) noexcept {
    if (check(type))
      return advance();

    throw RuntimeError(peek(), message);
  }

  inline expr::ExprPtr expression() noexcept {
    // expression -> equality ;

    return equality();
  }

  inline expr::ExprPtr equality() noexcept {
    // equality -> comparison ( ( "!=" | "==" ) comparison )* ;

    expr::ExprPtr left = comparison();
    while (match({TokenType::TK_NE, TokenType::TK_EQEQ})) {
      Token oper = prev();
      expr::ExprPtr right = comparison();
      left = std::make_shared<expr::Binary>(left, oper, right);
    }
    return left;
  }

  inline expr::ExprPtr comparison() noexcept {
    // comparison -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;

    expr::ExprPtr left = term();
    while (match({TokenType::TK_GT, TokenType::TK_GE, TokenType::TK_LT, TokenType::TK_LE})) {
      Token oper = prev();
      expr::ExprPtr right = term();
      left = std::make_shared<expr::Binary>(left, oper, right);
    }
    return left;
  }

  inline expr::ExprPtr term() noexcept {
    // term -> factor ( ( "-" | "+" ) factor )* ;

    expr::ExprPtr left = factor();
    while (match({TokenType::TK_MINUS, TokenType::TK_PLUS})) {
      Token oper = prev();
      expr::ExprPtr right = factor();
      left = std::make_shared<expr::Binary>(left, oper, right);
    }
    return left;
  }

  inline expr::ExprPtr factor() noexcept {
    // factor -> unary ( ( "/" | "*" ) unary )* ;

    expr::ExprPtr left = unary();
    while (match({TokenType::TK_SLASH, TokenType::TK_STAR})) {
      Token oper = prev();
      expr::ExprPtr right = unary();
      left = std::make_shared<expr::Binary>(left, oper, right);
    }
    return left;
  }

  inline expr::ExprPtr unary() noexcept {
    // unary -> ( "!" | "-" ) unary | primary ;

    if (match({TokenType::TK_NOT, TokenType::TK_MINUS})) {
      TokenType oper = prev();
      expr::ExprPtr right = unary();
      return std::make_shared<expr::Unary>(oper, right);
    }
    return primary();
  }

  inline expr::ExprPtr primary() noexcept {
    // primary -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;

    if (match({TokenType::KW_TRUE}))
      return std::make_shared<expr::Literal>(true);
    if (match({TokenType::KW_FALSE}))
      return std::make_shared<expr::Literal>(false);
    if (match({TokenType::KW_NIL}))
      return std::make_shared<expr::Literal>(nullptr);

    if (match({TokenType::TK_NUMERIC}))
      return std::make_shared<expr::Literal>(prev().as_numeric());
    if (match({TokenType::TK_STRING}))
      return std::make_shared<expr::Literal>(prev().as_string());

    if (match({TokenType::TK_LPAREN})) {
      expr::ExprPtr expr = expression();
      consume(TokenType::TK_RPAREN, "Expect `)` after expression.");
      return std::make_shared<expr::Grouping>(expr);
    }
    return nullptr;
  }
public:
  Parser(const std::vector<Token>& tokens) noexcept
    : tokens_{tokens} {
  }
};

}
