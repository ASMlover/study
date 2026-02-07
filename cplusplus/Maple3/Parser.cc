// Copyright (c) 2026 ASMlover. All rights reserved.
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
#include "Parser.hh"

namespace ms {

Parser::Parser(const std::vector<Token>& tokens, const str_t& source_fname) noexcept
  : tokens_{tokens}, source_fname_{source_fname} {
}

bool Parser::is_at_end() const noexcept {
  return peek().type() == TokenType::TK_EOF;
}

const Token& Parser::peek() const noexcept {
  return tokens_[current_];
}

const Token& Parser::previous() const noexcept {
  return tokens_[current_ - 1];
}

const Token& Parser::advance() noexcept {
  if (!is_at_end())
    ++current_;
  return previous();
}

bool Parser::check(TokenType type) const noexcept {
  return !is_at_end() ? peek().type() == type : false;
}

bool Parser::match(std::initializer_list<TokenType> types) noexcept {
  for (auto type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

ParseError Parser::parse_error(const Token& token, const str_t& message) noexcept {
  return ParseError{message, source_fname_, token.lineno(), token.colno()};
}

void Parser::synchronize() noexcept {
  advance();

  while (!is_at_end()) {
    if (previous().type() == TokenType::TK_SEMICOLON)
      return;

    switch (peek().type()) {
    case TokenType::KW_CLASS:
    case TokenType::KW_FUN:
    case TokenType::KW_VAR:
    case TokenType::KW_FOR:
    case TokenType::KW_IF:
    case TokenType::KW_WHILE:
    case TokenType::KW_PRINT:
    case TokenType::KW_RETURN:
      return;
    default:
      break;
    }

    advance();
  }
}

const Token& Parser::consume(TokenType type, const str_t& message) {
  if (check(type))
    return advance();
  throw parse_error(peek(), message);
}

ExprPtr Parser::expression() {
  return equality();
}

ExprPtr Parser::equality() {
  ExprPtr expr = comparison();
  while (match({TokenType::TK_BANG_EQUAL, TokenType::TK_EQUAL_EQUAL})) {
    Token op = previous();
    ExprPtr right = comparison();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::comparison() {
  ExprPtr expr = term();
  while (match({TokenType::TK_GREATER, TokenType::TK_GREATER_EQUAL, TokenType::TK_LESS, TokenType::TK_LESS_EQUAL})) {
    Token op = previous();
    ExprPtr right = term();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::term() {
  ExprPtr expr = factor();
  while (match({TokenType::TK_MINUS, TokenType::TK_PLUS})) {
    Token op = previous();
    ExprPtr right = factor();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::factor() {
  ExprPtr expr = unary();
  while (match({TokenType::TK_SLASH, TokenType::TK_STAR})) {
    Token op = previous();
    ExprPtr right = unary();
    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
  }
  return expr;
}

ExprPtr Parser::unary() {
  if (match({TokenType::TK_BANG, TokenType::TK_MINUS})) {
    Token op = previous();
    ExprPtr right = unary();
    return std::make_unique<UnaryExpr>(op, std::move(right));
  }

  return primary();
}

ExprPtr Parser::primary() {
  if (match({TokenType::KW_FALSE, TokenType::KW_TRUE, TokenType::KW_NIL, TokenType::TK_NUMBER, TokenType::TK_STRING}))
    return std::make_unique<LiteralExpr>(previous());

  if (match({TokenType::TK_LPAREN})) {
    ExprPtr expr = expression();
    consume(TokenType::TK_RPAREN, "Expected ')' after expression.");
    return std::make_unique<GroupingExpr>(std::move(expr));
  }

  throw parse_error(peek(), "Expected expression.");
}

ExprPtr Parser::parse_expression() noexcept {
  try {
    return expression();
  }
  catch (const ParseError& error) {
    errors_.push_back(error.what());
  }
  return nullptr;
}

ExprList Parser::parse_program() noexcept {
  ExprList exprs;

  while (!is_at_end()) {
    try {
      ExprPtr expr = expression();
      if (expr)
        exprs.push_back(std::move(expr));

      if (match({TokenType::TK_SEMICOLON}))
        continue;

      if (!is_at_end())
        throw parse_error(peek(), "Expected ';' after expression.");
    }
    catch (const ParseError& error) {
      errors_.push_back(error.what());
      synchronize();
    }
  }

  return exprs;
}

}
