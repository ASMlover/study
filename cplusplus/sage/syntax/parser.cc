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
#include "../common/errors.h"
#include "../lex/token.h"
#include "parser.hh"

namespace sage {

Parser::Parser(ErrorReport& err_report, const std::vector<Token>& tokens)
  : err_report_(err_report), tokens_(tokens) {
}

ExprPtr Parser::parse(void) {
  try {
    return expression();
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.get_token(), e.get_message());
    return nullptr;
  }
}

std::vector<StmtPtr> Parser::parse_stmts(void) {
  // program -> declaration* EOF ;

  std::vector<StmtPtr> stmts;
  while (!is_end())
    stmts.push_back(declaration());
  return stmts;
}

bool Parser::is_end(void) const {
  return peek().get_kind() == TokenKind::TK_EOF;
}

const Token& Parser::peek(void) const {
  return tokens_[curpos_];
}

const Token& Parser::prev(void) const {
  return tokens_[curpos_ - 1];
}

const Token& Parser::advance(void) {
  if (!is_end())
    ++curpos_;
  return prev();
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

void Parser::synchronize(void) {
  advance();

  while (!is_end()) {
    auto prev_kind = prev().get_kind();
    if (prev_kind == TokenKind::TK_NL)
      return;

    switch (prev_kind) {
    case TokenKind::KW_IF:
    case TokenKind::KW_WHILE:
    case TokenKind::KW_FOR:
    case TokenKind::KW_BREAK:
    case TokenKind::KW_FN:
    case TokenKind::KW_RETURN:
    case TokenKind::KW_CLASS:
    case TokenKind::KW_LET:
    case TokenKind::KW_IMPORT:
    case TokenKind::KW_PRINT:
      return;
    }
  }
}

StmtPtr Parser::declaration(void) {
  // declaration -> let_decl | statement ;

  try {
    if (match({TokenKind::KW_LET}))
      return let_decl();

    return statement();
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.get_token(), e.get_message());
    synchronize();

    return nullptr;
  }
}

StmtPtr Parser::let_decl(void) {
  // let_decl -> "let" IDENTIFIER ( "=" expression )? NL ;

  const Token& name = consume(TokenKind::TK_IDENTIFIER, "expect variable name");
  ExprPtr expr;
  if (match({TokenKind::TK_EQUAL}))
    expr = expression();
  consume(TokenKind::TK_NL, "expect `NL` after varibale declaration");
  return std::make_shared<LetStmt>(name, expr);
}

StmtPtr Parser::statement(void) {
  // statement -> print_stmt | expr_stmt ;

  if (match({TokenKind::KW_PRINT}))
    return print_stmt();
  return expr_stmt();
}

StmtPtr Parser::print_stmt(void) {
  // print_stmt -> "print" ( expression ( "," expression )* )? NL ;

  std::vector<ExprPtr> exprs;
  if (!match({TokenKind::TK_NL})) {
    do {
      exprs.push_back(expression());
    } while (match({TokenKind::TK_COMMA}));
    consume(TokenKind::TK_NL, "expect `NL` after `print` expressions");
  }
  return std::make_shared<PrintStmt>(exprs);
}

StmtPtr Parser::expr_stmt(void) {
  // expr_stmt -> expression NL ;

  ExprPtr expr = expression();
  consume(TokenKind::TK_NL, "expect `NL` after expression");
  return std::make_shared<ExprStmt>(expr);
}

ExprPtr Parser::expression(void) {
  // expression -> assignment ;

  return assignment();
}

ExprPtr Parser::assignment(void) {
  // assignment   -> IDENTIFIER ( assign_oper ) assignment | equality ;
  // assign_oper  -> "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;

  ExprPtr expr = equality();
  if (match({TokenKind::TK_EQUAL, TokenKind::TK_PLUSEQUAL,
        TokenKind::TK_MINUSEQUAL, TokenKind::TK_STAREQUAL,
        TokenKind::TK_SLASHEQUAL, TokenKind::TK_PERCENTEQUAL})) {
    const Token& oper = prev();
    ExprPtr value = assignment();
    if (std::dynamic_pointer_cast<VariableExpr>(expr)) {
      const Token& name = std::static_pointer_cast<VariableExpr>(expr)->name();
      return std::make_shared<AssignExpr>(name, oper, value);
    }

    throw RuntimeError(oper, "invalid assignment target");
  }
  return expr;
}

ExprPtr Parser::equality(void) {
  // equality -> comparison ( ( "not" | "!=" | "==" ) comparison )* ;

  ExprPtr expr = comparison();
  while (match({TokenKind::KW_IS,
        TokenKind::TK_EXCLAIMEQUAL, TokenKind::TK_EQUALEQUAL})) {
    const Token& oper = prev();
    ExprPtr right = comparison();
    expr = std::make_shared<BinaryExpr>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::comparison(void) {
  // comparison -> addition ( ( ">" | ">=" | "<" | "<=" ) addition )* ;

  ExprPtr expr = addition();
  while (match({TokenKind::TK_LESS, TokenKind::TK_LESSEQUAL,
        TokenKind::TK_GREATER, TokenKind::TK_GREATEREQUAL})) {
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
  // unary -> ( "not" | "!" | "-" ) unary | primary ;

  if (match({TokenKind::KW_NOT, TokenKind::TK_EXCLAIM, TokenKind::TK_MINUS})) {
    const Token& oper = prev();
    ExprPtr right = unary();
    return std::make_shared<UnaryExpr>(oper, right);
  }
  return primary();
}

ExprPtr Parser::primary(void) {
  // primary -> INTEGER | DECIMAL | STRICT | "true" | "false" | "nil"
  //          | "(" expression ")" | IDENTIFIER ;

  if (match({TokenKind::TK_INTEGERCONST}))
    return std::make_shared<LiteralExpr>(prev().as_integer());
  if (match({TokenKind::TK_DECIMALCONST}))
    return std::make_shared<LiteralExpr>(prev().as_decimal());
  if (match({TokenKind::TK_STRINGLITERAL}))
    return std::make_shared<LiteralExpr>(prev().as_string());
  if (match({TokenKind::KW_TRUE}))
    return std::make_shared<LiteralExpr>(true);
  if (match({TokenKind::KW_FALSE}))
    return std::make_shared<LiteralExpr>(false);
  if (match({TokenKind::KW_NIL}))
    return std::make_shared<LiteralExpr>(nullptr);

  if (match({TokenKind::TK_LPAREN})) {
    ExprPtr expr = expression();
    consume(TokenKind::TK_RPAREN, "expect `)` after expression");
    return std::make_shared<GroupingExpr>(expr);
  }

  if (match({TokenKind::TK_IDENTIFIER}))
    return std::make_shared<VariableExpr>(prev());

  throw RuntimeError(peek(), "expect expression");
}

}
