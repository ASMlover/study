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
#include "token.h"
#include "errors.h"
#include "parser.h"

namespace lox {

ExprPtr Parser::parse(void) {
  try {
    return expression();
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.get_token(), e.get_message());
    return nullptr;
  }
}

std::vector<StmtPtr> Parser::parse_stmt(void) {
  std::vector<StmtPtr> stmts;
  while (!is_end())
    stmts.push_back(statement());

  return stmts;
}

bool Parser::is_end(void) const {
  return peek().get_kind() == TokenKind::TK_EOF;
}

const Token& Parser::advance(void) {
  if (!is_end())
    ++curpos_;
  return prev();
}

const Token& Parser::peek(void) const {
  return tokens_[curpos_];
}

const Token& Parser::prev(void) const {
  return tokens_[curpos_ - 1];
}

bool Parser::check(TokenKind kind) const {
  return is_end() ? false : peek().get_kind() == kind;
}

bool Parser::match(const std::initializer_list<TokenKind>& kinds) {
  for (auto kind : kinds) {
    if (check(kind)) {
      advance();
      return true;
    }
  }
  return false;
}

const Token& Parser::consume(TokenKind kind, const std::string& message) {
  if (check(kind))
    return advance();

  throw RuntimeError(peek(), message);
}

const Token& Parser::consume(
    const std::initializer_list<TokenKind>& kinds, const std::string& message) {
  for (auto kind : kinds) {
    if (check(kind))
      return advance();
  }

  throw RuntimeError(peek(), message);
}

ExprPtr Parser::expression(void) {
  // expression -> equality ;

  return equality();
}

ExprPtr Parser::equality(void) {
  // equality -> comparison ( ( "is" | "==" | "!=" ) comparison )* ;

  ExprPtr expr = comparison();
  while (match({TokenKind::KW_IS,
        TokenKind::TK_EQUALEQUAL, TokenKind::TK_EXCLAIMEQUAL})) {
    const Token& oper = prev();
    ExprPtr right = comparison();
    expr = std::make_shared<BinaryExpr>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::comparison(void) {
  // comparison -> addition ( ( ">" | ">=" | "<" | "<=" ) addition )* ;

  ExprPtr expr = addition();
  while (match({TokenKind::TK_GREATER, TokenKind::TK_GREATEREQUAL,
        TokenKind::TK_LESS, TokenKind::TK_LESSEQUAL})) {
    const Token& oper = prev();
    ExprPtr right = addition();
    expr = std::make_shared<BinaryExpr>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::addition(void) {
  // addition -> multiplication ( ( "+" | "-" ) multiplication )* ;

  ExprPtr expr = multiplication();
  while (match({TokenKind::TK_PLUS, TokenKind::TK_MINUS})) {
    const Token& oper = prev();
    ExprPtr right = multiplication();
    expr = std::make_shared<BinaryExpr>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::multiplication(void) {
  // multiplication -> unary ( ( "*" | "/" | "%" ) unary )* ;

  ExprPtr expr = unary();
  while (match({TokenKind::TK_STAR,
        TokenKind::TK_SLASH, TokenKind::TK_PERCENT})) {
    const Token& oper = prev();
    ExprPtr right = unary();
    expr = std::make_shared<BinaryExpr>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::unary(void) {
  // unary -> ( "-" | "!" | "not" ) unary | primary ;

  if (match({TokenKind::TK_MINUS, TokenKind::TK_EXCLAIM, TokenKind::KW_NOT})) {
    const Token& oper = prev();
    ExprPtr right = unary();
    return std::make_shared<UnaryExpr>(oper, right);
  }

  return primary();
}

ExprPtr Parser::primary(void) {
  // primary  -> INTEGER | DECIMAL | STRING | "true" | "false" | "nil"
  //          | "(" expression ")" ;

  if (match({TokenKind::KW_TRUE}))
    return std::make_shared<LiteralExpr>(true);
  if (match({TokenKind::KW_FALSE}))
    return std::make_shared<LiteralExpr>(false);
  if (match({TokenKind::KW_NIL}))
    return std::make_shared<LiteralExpr>(nullptr);

  if (match({TokenKind::TK_INTEGERCONST}))
    return std::make_shared<LiteralExpr>(prev().as_integer());
  if (match({TokenKind::TK_DECIMALCONST}))
    return std::make_shared<LiteralExpr>(prev().as_decimal());
  if (match({TokenKind::TK_STRINGLITERAL}))
    return std::make_shared<LiteralExpr>(prev().as_string());

  if (match({TokenKind::TK_LPAREN})) {
    ExprPtr expr = expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after expression ...");
    return std::make_shared<GroupingExpr>(expr);
  }

  throw RuntimeError(peek(), "expect expression ...");
}

StmtPtr Parser::statement(void) {
  // statement -> expr_stmt | print_stmt ;

  if (match({TokenKind::KW_PRINT}))
    return print_stmt();
  return expr_stmt();
}

StmtPtr Parser::expr_stmt(void) {
  // expr_stmt -> expression NEWLINE ;

  ExprPtr expr = expression();
  consume(TokenKind::TK_NEWLINE, "expect `NL` after expression ...");
  return std::make_shared<ExprStmt>(expr);
}

StmtPtr Parser::print_stmt(void) {
  // print_stmt -> "print" expression NEWLINE ;

  ExprPtr expr = expression();
  consume(TokenKind::TK_NEWLINE, "expect `NL` after `print` expression ...");
  return std::make_shared<PrintStmt>(expr);
}

}
