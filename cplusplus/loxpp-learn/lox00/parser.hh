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
#include "stmt.hh"

namespace loxpp::parser {

// program      -> declaration* EOF ;
// declaration  -> funDecl | varDecl | statement ;
// funDecl      -> "fun" function ;
// function     -> IDENTIFIER "(" parameters? ")" block ;
// parameters   -> IDENTIFIER ( "," IDENTIFIER )* ;
// varDecl      -> "var" IDENTIFIER ( "=" expression )? ";" ;
// statement    -> exprStmt | ifStmt | whileStmt | forStmt | printStmt | block ;
// exprStmt     -> expression ";" ;
// ifStmt       -> "if" "(" expression ")" statement ( "else" statement )? ;
// whileStmt    -> "while" "(" expression ")" statement ;
// forStmt      -> "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;
// printStmt    -> "print" expression ";" ;
// block        -> "{" declaration* "}" ;
// expression   -> assignment ;
// assignment   -> IDENTIFIER  "=" assignment | logic_or ;
// logic_or     -> logic_and ( "or" logic_and )* ;
// logic_and    -> equality ( "and" equality )* ;
// equality     -> comparison ( ( "!=" | "==" ) comparison )* ;
// comparison   -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
// term         -> factor ( ( "-" | "+" ) factor )* ;
// factor       -> unary ( ( "/" | "*" ) unary )* ;
// unary        -> ( "!" | "-" ) unary | call ;
// call         -> primary ( "(" arguments? ")" )* ;
// arguments    -> expression ( "," expression )* ;
// primary      -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" | IDENTIFIER ;

class ParserError final : public Copyable, public std::exception {};

class Parser final : private UnCopyable {
  ErrorReporter& err_reporter_;
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

  Token consume(TokenType type, const str_t& message) {
    if (check(type))
      return advance();

    throw error(peek(), message);
  }

  inline ParserError error(const Token& token, const str_t& message) noexcept {
    err_reporter_.error(token, message);
    return ParserError();
  }

  void synchronize() {
    advance();

    while (!is_at_end()) {
      if (prev().type() == TokenType::TK_SEMI)
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
      }

      advance();
    }
  }

  inline stmt::StmtPtr declaration() noexcept {
    // declaration -> funDecl | varDecl | statement ;

    try {
      if (match({TokenType::KW_FUN}))
        return function("function");
      if (match({TokenType::KW_VAR}))
        return var_declaration();
      return statement();
    }
    catch (const ParserError&) {
      synchronize();
      return nullptr;
    }
  }

  inline stmt::FunctionPtr function(const str_t& kind) noexcept {
    // function -> IDENTIFIER "(" parameters? ")" block ;
    // TODO:

    Token name = consume(TokenType::TK_IDENTIFIER, "expect " + kind + " name");
    return nullptr;
  }

  inline stmt::StmtPtr var_declaration() noexcept {
    // varDecl -> "var" IDENTIFIER ( "=" expression )? ";" ;

    Token name = consume(TokenType::TK_IDENTIFIER, "expect variable name");

    expr::ExprPtr initializer = nullptr;
    if (match({TokenType::TK_EQ}))
      initializer = expression();
    consume(TokenType::TK_SEMI, "expect `;` after variable declaration");
    return std::make_shared<stmt::Var>(name, initializer);
  }

  inline stmt::StmtPtr statement() noexcept {
    // statement -> exprStmt | printStmt | block ;

    if (match({TokenType::KW_IF}))
      return if_statement();
    if (match({TokenType::KW_WHILE}))
      return while_statement();
    if (match({TokenType::KW_FOR}))
      return for_statement();
    if (match({TokenType::KW_PRINT}))
      return print_statement();
    if (match({TokenType::TK_LBRACE}))
      return std::make_shared<stmt::Block>(block());
    return expression_statement();
  }

  inline stmt::StmtPtr print_statement() noexcept {
    // printStmt -> "print" expression ";" ;

    expr::ExprPtr value = expression();
    consume(TokenType::TK_SEMI, "expect `;` after value ...");
    return std::make_shared<stmt::Print>(value);
  }

  inline std::vector<stmt::StmtPtr> block() noexcept {
    // block -> "{" declaration* "}" ;

    std::vector<stmt::StmtPtr> statements;
    while (!check(TokenType::TK_RBRACE) && !is_at_end())
      statements.emplace_back(declaration());

    consume(TokenType::TK_RBRACE, "expect `}` after block");
    return statements;
  }

  inline stmt::StmtPtr expression_statement() noexcept {
    // exprStmt -> expression ;

    expr::ExprPtr expr = expression();
    consume(TokenType::TK_SEMI, "expect `;` after expression ...");
    return std::make_shared<stmt::Expression>(expr);
  }

  inline stmt::StmtPtr if_statement() noexcept {
    // ifStmt -> "if" "(" expression ")" statement ( "else" statement )? ;

    consume(TokenType::TK_LPAREN, "expect `(` after `if`");
    expr::ExprPtr condition = expression();
    consume(TokenType::TK_RPAREN, "expect `)` after if condition");

    stmt::StmtPtr then_branch = statement();
    stmt::StmtPtr else_branch = nullptr;
    if (match({TokenType::KW_ELSE}))
      else_branch = statement();

    return std::make_shared<stmt::If>(condition, then_branch, else_branch);
  }

  inline stmt::StmtPtr while_statement() noexcept {
    // whileStmt -> "while" "(" expression ")" statement ;

    consume(TokenType::TK_LPAREN, "expect `(` after `while`");
    expr::ExprPtr cond = expression();
    consume(TokenType::TK_RPAREN, "expect `)` after while condition");
    stmt::StmtPtr body = statement();

    return std::make_shared<stmt::While>(cond, body);
  }

  inline stmt::StmtPtr for_statement() noexcept {
    // forStmt -> "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement ;

    consume(TokenType::TK_LPAREN, "expect `(` after `for`");
    stmt::StmtPtr initializer;
    if (match({TokenType::TK_SEMI}))
      initializer = nullptr;
    else if (match({TokenType::KW_VAR}))
      initializer = var_declaration();
    else
      initializer = expression_statement();

    expr::ExprPtr condition;
    if (!check(TokenType::TK_SEMI))
      condition = expression();
    consume(TokenType::TK_SEMI, "expect `;` after loop condition");

    expr::ExprPtr increment;
    if (!check(TokenType::TK_RPAREN))
      increment = expression();
    consume(TokenType::TK_RPAREN, "expect `)` after for clauses");

    stmt::StmtPtr body = statement();
    if (increment) {
      std::vector<stmt::StmtPtr> stmts{body, std::make_shared<stmt::Expression>(increment)};
      body = std::make_shared<stmt::Block>(stmts);
    }
    if (!condition)
      condition = std::make_shared<expr::Literal>(true);
    body = std::make_shared<stmt::While>(condition, body);

    if (!initializer) {
      std::vector<stmt::StmtPtr> stmts{initializer, body};
      body = std::make_shared<stmt::Block>(stmts);
    }

    return body;
  }

  inline expr::ExprPtr expression() noexcept {
    // expression -> assignment ;

    return assignment();
  }

  inline expr::ExprPtr assignment() noexcept {
    // assignment -> IDENTIFIER "=" assignment | logic_or ;

    expr::ExprPtr expr = equality();
    if (match({TokenType::TK_EQ})) {
      const Token& equals = prev();
      expr::ExprPtr value = assignment();

      if (std::dynamic_pointer_cast<expr::Variable>(expr)) {
        const Token& name = std::static_pointer_cast<expr::Variable>(expr)->name();
        return std::make_shared<expr::Assign>(name, value);
      }

      error(equals, "invalid assignment target");
    }
    return expr;
  }

  inline expr::ExprPtr logic_or() noexcept {
    // logic_or -> logic_and ( "or" logic_and )* ;

    expr::ExprPtr expr = logic_and();
    while (match({TokenType::KW_OR})) {
      Token oper = prev();
      expr::ExprPtr right = logic_and();
      expr = std::make_shared<expr::Logical>(expr, oper, right);
    }

    return expr;
  }

  inline expr::ExprPtr logic_and() noexcept {
    // logic_and -> equality ( "and" equality )* ;

    expr::ExprPtr expr = equality();
    while (match({TokenType::KW_AND})) {
      Token oper = prev();
      expr::ExprPtr right = equality();
      expr = std::make_shared<expr::Logical>(expr, oper, right);
    }

    return expr;
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
      Token oper = prev();
      expr::ExprPtr right = unary();
      return std::make_shared<expr::Unary>(oper, right);
    }
    return primary();
  }

  inline expr::ExprPtr call() noexcept {
    // call -> primary ( "(" arguments? ")" )* ;

    expr::ExprPtr expr = primary();
    while (true) {
      if (match({TokenType::TK_LPAREN}))
        expr = finish_call(expr);
      else
        break;
    }

    return expr;
  }

  inline expr::ExprPtr finish_call(const expr::ExprPtr& callee)  noexcept {
    std::vector<expr::ExprPtr> arguments;
    if (!check(TokenType::TK_LPAREN)) {
      do {
        if (arguments.size() >= 255)
          error(peek(), "cannot have more than 255 arguments");
        arguments.push_back(expression());
      } while (match({TokenType::TK_COMMA}));
    }
    Token paren = consume(TokenType::TK_RPAREN, "expect `)` after arguments");

    return std::make_shared<expr::Call>(callee, paren, arguments);
  }

  inline expr::ExprPtr primary() noexcept(false) {
    // primary -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" | IDENTIFIER ;

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

    if (match({TokenType::TK_IDENTIFIER}))
      return std::make_shared<expr::Variable>(prev());

    if (match({TokenType::TK_LPAREN})) {
      expr::ExprPtr expr = expression();
      consume(TokenType::TK_RPAREN, "Expect `)` after expression.");
      return std::make_shared<expr::Grouping>(expr);
    }

    throw error(peek(), "Expect expression.");
  }
public:
  Parser(ErrorReporter& err_reporter, const std::vector<Token>& tokens) noexcept
    : err_reporter_{err_reporter}, tokens_{tokens} {
  }

  inline std::vector<stmt::StmtPtr> parse() noexcept {
    // program -> declaration* EOF ;

    std::vector<stmt::StmtPtr> statements;
    while (!is_at_end())
      statements.push_back(declaration());
    return statements;
  }
};

}
