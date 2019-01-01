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

std::vector<StmtPtr> Parser::parse_stmt(void) {
  std::vector<StmtPtr> stmts;
  while (!is_end())
    stmts.push_back(declaration());

  return stmts;
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
  // expression -> assignment;

  return assignment();
}

ExprPtr Parser::assignment(void) {
  // assignment -> IDENTIFILER "=" assignment | logical_or;

  ExprPtr expr = logical_or();
  if (match({TOKEN_EQUAL})) {
    Token equals = prev();
    ExprPtr value = assignment();

    if (std::dynamic_pointer_cast<Variable>(expr)) {
      Token name = std::static_pointer_cast<Variable>(expr)->name_;
      return std::make_shared<Assign>(name, value);
    }

    this->error(equals, "invalid assignment target ...");
  }

  return expr;
}

ExprPtr Parser::logical_or(void) {
  // logical_or -> logical_and ( "or" logical_and )* ;

  ExprPtr expr = logical_and();
  while (match({TOKEN_OR})) {
    Token oper = prev();
    ExprPtr right = logical_and();
    expr = std::make_shared<Logical>(expr, oper, right);
  }

  return expr;
}

ExprPtr Parser::logical_and(void) {
  // logical_and -> equality ( "and" equality )* ;

  ExprPtr expr = equality();
  while (match({TOKEN_AND})) {
    Token oper = prev();
    ExprPtr right = equality();
    expr = std::make_shared<Logical>(expr, oper, right);
  }

  return expr;
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
  // primary -> NUMBER | STRING | "true" | "false" | "nil" | "this"
  //            |"(" expression ")" | IDENTIFILER;

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
  if (match({TOKEN_IDENTIFIER}))
    return std::make_shared<Variable>(prev());

  if (match({TOKEN_LEFT_PAREN})) {
    auto expr = expression();
    consume(TOKEN_RIGHT_PAREN, "expected `)` after expression ...");
    return std::make_shared<Grouping>(expr);
  }

  throw error(peek(), "expect expression in primary ...");
}

StmtPtr Parser::declaration(void) {
  try {
    if (match({TOKEN_VAR}))
      return var_declaration();

    return statement();
  }
  catch (const ParseError&) {
    synchronize();
    return nullptr;
  }
}

StmtPtr Parser::var_declaration(void) {
  Token name = consume(TOKEN_IDENTIFIER, "expect variable name ...");
  ExprPtr init = nullptr;
  if (match({TOKEN_EQUAL}))
    init = expression();
  consume(TOKEN_SEMICOLON, "expect `;` after variable declaration ...");

  return std::make_shared<VarStmt>(name, init);
}

StmtPtr Parser::statement(void) {
  // statement -> expr_stmt
  //              | for_stmt
  //              | if_stmt
  //              | print_stmt
  //              | while_stmt
  //              | block;

  if (match({TOKEN_FOR}))
    return for_statement();
  if (match({TOKEN_IF}))
    return if_statement();
  if (match({TOKEN_PRINT}))
    return print_statement();
  if (match({TOKEN_WHILE}))
    return while_statement();
  if (match({TOKEN_LEFT_BRACE}))
    return std::make_shared<BlockStmt>(block());
  return expression_statement();
}

StmtPtr Parser::for_statement(void) {
  // for_stmt -> "for" "(" (var_decl | expr_stmt | ";")
  //                        expression? ";"
  //                        expression? ")" statement ;

  consume(TOKEN_LEFT_PAREN, "expect `(` after `for` ...");

  StmtPtr init;
  if (match({TOKEN_SEMICOLON}))
    init = nullptr;
  else if (match({TOKEN_VAR}))
    init = var_declaration();
  else
    init = expression_statement();
  ExprPtr cond;
  if (!check(TOKEN_SEMICOLON))
    cond = expression();
  consume(TOKEN_SEMICOLON, "expect `;` after for loop condition ...");
  ExprPtr step;
  if (!check(TOKEN_RIGHT_PAREN))
    step = expression();
  consume(TOKEN_RIGHT_PAREN, "expect `)` after for loop clauses ...");

  StmtPtr body = statement();
  if (step) {
    body = std::make_shared<BlockStmt>(
        std::vector<StmtPtr>{body, std::make_shared<ExprStmt>(step)});
  }
  if (!cond)
    cond = std::make_shared<Literal>(true);
  body = std::make_shared<WhileStmt>(cond, body);
  if (init)
    body = std::make_shared<BlockStmt>(std::vector<StmtPtr>{init, body});

  return body;
}

StmtPtr Parser::if_statement(void) {
  // if_stmt -> "if" "(" expression ")" statement ( "else" statement)? ;

  consume(TOKEN_LEFT_PAREN, "expect `(` after `if` ...");
  ExprPtr cond = expression();
  consume(TOKEN_RIGHT_PAREN, "expect `)` after if condition ...");

  StmtPtr then_branch = statement();
  StmtPtr else_branch;
  if (match({TOKEN_ELSE}))
    else_branch = statement();

  return std::make_shared<IfStmt>(cond, then_branch, else_branch);
}

StmtPtr Parser::print_statement(void) {
  auto value = expression();
  consume(TOKEN_SEMICOLON, "expect `;` after value ...");
  return std::make_shared<PrintStmt>(value);
}

StmtPtr Parser::while_statement(void) {
  // while_stmt -> "while" "(" expression ")" statement;

  consume(TOKEN_LEFT_PAREN, "expect `(` after `while` ...");
  ExprPtr cond = expression();
  consume(TOKEN_RIGHT_PAREN, "expect `)` after while condition ...");

  StmtPtr body = statement();
  return std::make_shared<WhileStmt>(cond, body);
}

std::vector<StmtPtr> Parser::block(void) {
  // block -> "{" declaration* "}";

  std::vector<StmtPtr> stmts;
  while (!check(TOKEN_RIGHT_BRACE) && !is_end())
    stmts.push_back(declaration());
  consume(TOKEN_RIGHT_BRACE, "expect `}` after block ...");
  return stmts;
}

StmtPtr Parser::expression_statement(void) {
  auto expr = expression();
  consume(TOKEN_SEMICOLON, "expect `;` after expression ...");
  return std::make_shared<ExprStmt>(expr);
}
