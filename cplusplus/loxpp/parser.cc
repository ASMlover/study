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
#include <iostream>
#include "parser.h"

Parser::Parser(const std::vector<Token>& tokens)
  : tokens_(tokens) {
}

ExprPtr Parser::parse(void) {
  try {
    return expression();
  } catch (const ParseError&) {
    return nullptr;
  }
}

bool Parser::is_end(void) const {
  return peek().get_type() == TOKEN_EOF;
}

Token Parser::peek(void) const {
  return tokens_[current_];
}

Token Parser::prev(void) const {
  return tokens_[current_ - 1];
}

Token Parser::advance(void) {
  if (!is_end())
    ++current_;
  return prev();
}

bool Parser::check(TokenType type) const {
  return is_end() ? false : peek().get_type() == type;
}

Token Parser::consume(TokenType type, const std::string& msg) {
  if (check(type))
    return advance();

  throw error(peek(), msg);
}

ParseError Parser::error(const Token& tok, const std::string& msg) {
  std::cerr << tok.repr() << " - " << msg << std::endl;
  return ParseError();
}

bool Parser::match(const std::initializer_list<TokenType>& types) {
  for (auto t : types) {
    if (check(t)) {
      advance();
      return true;
    }
  }
  return false;
}

void Parser::synchronize(void) {
  advance();

  while (!is_end()) {
    if (prev().get_type() == TOKEN_SEMICOLON)
      return;

    auto tok = peek();
    switch (tok.get_type()) {
    case TOKEN_CLASS:
    case TOKEN_FUN:
    case TOKEN_VAR:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_PRINT:
    case TOKEN_RETURN:
      return;
    }

    advance();
  }
}

ExprPtr Parser::expression(void) {
  return equality();
}

ExprPtr Parser::equality(void) {
  // equality -> comparison (("!="|"==") comparison)*;

  auto expr = comparison();
  while (match({TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL})) {
    auto oper = prev();
    auto right = comparison();
    expr = std::make_shared<Binary>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::comparison(void) {
  // comparison -> addition ((">"|">="|"<"|"<=") addition)*;

  auto expr = addition();
  while (match({TOKEN_GREATER,
        TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL})) {
    auto oper = prev();
    auto right = addition();
    expr = std::make_shared<Binary>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::addition(void) {
  auto expr = multiplication();
  while (match({TOKEN_PLUS, TOKEN_MINUS})) {
    auto oper = prev();
    auto right = multiplication();
    expr = std::make_shared<Binary>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::multiplication(void) {
  auto expr = unary();
  while (match({TOKEN_STAR, TOKEN_SLASH})) {
    auto oper = prev();
    auto right = unary();
    expr = std::make_shared<Binary>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::unary(void) {
  // unary -> ("!"|"-") unary | primary;

  if (match({TOKEN_BANG, TOKEN_MINUS})) {
    auto oper = prev();
    auto right = unary();
    return std::make_shared<Unary>(oper, right);
  }
  return primary();
}

ExprPtr Parser::primary(void) {
  // primary -> NUMBER|STRING|"true"|"false"|"nil"|"(" expression ")";

  if (match({TOKEN_FALSE}))
    return std::make_shared<Literal>(Value(false));
  if (match({TOKEN_TRUE}))
    return std::make_shared<Literal>(Value(true));
  if (match({TOKEN_NIL}))
    return std::make_shared<Literal>(Value());
  if (match({TOKEN_NUMBER}))
    return std::make_shared<Literal>(Value(prev().as_number()));
  if (match({TOKEN_STRING}))
    return std::make_shared<Literal>(Value(prev().as_string()));

  if (match({TOKEN_LEFT_PAREN})) {
    auto expr = expression();
    consume(TOKEN_RIGHT_PAREN, "expected `)` after expression ...");
    return std::make_shared<Grouping>(expr);
  }

  throw error(peek(), "expect expression in primary ...");
}
