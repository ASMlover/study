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
#include "interpret_errors.hh"
#include "interpret_parser.hh"

namespace loxcc::interpret {

std::vector<StmtPtr> Parser::parse(void) {
  // program -> declaration* EOF;

  std::vector<StmtPtr> stmts;

  advance();
  while (!is_end())
    stmts.push_back(declaration());

  return stmts;
}

Token Parser::advance(void) {
  if (!is_end()) {
    prev_ = curr_;
    curr_ = lex_.next_token();

    if (curr_.kind() == TokenKind::TK_ERR)
      throw RuntimeError(curr_, curr_.literal());
  }
  return prev_;
}

bool Parser::check(TokenKind kind) const {
  return is_end() ? false : curr_.kind() == kind;
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

Token Parser::consume(TokenKind kind, const str_t& message) {
  if (check(kind))
    return advance();

  throw RuntimeError(curr_, message);
}

void Parser::synchronize(void) {
  advance();

  while (!is_end()) {
    if (prev_.kind() == TokenKind::TK_SEMI)
      return;

    switch (curr_.kind()) {
    case TokenKind::KW_CLASS:
    case TokenKind::KW_FUN:
    case TokenKind::KW_FOR:
    case TokenKind::KW_IF:
    case TokenKind::KW_PRINT:
    case TokenKind::KW_RETURN:
    case TokenKind::KW_VAR:
    case TokenKind::KW_WHILE:
      return;
    }

    advance();
  }
}

StmtPtr Parser::declaration(void) {
  // declaration -> class_decl | fun_decl | var_decl | statement ;

  try {
    if (match({TokenKind::KW_CLASS}))
      return class_decl();
    if (match({TokenKind::KW_FUN}))
      return fun_decl("function");
    if (match({TokenKind::KW_VAR}))
      return var_decl();

    return statement();
  }
  catch (const RuntimeError& e) {
    err_report_.error(e.token(), e.message());
    synchronize();

    return nullptr;
  }
}

StmtPtr Parser::class_decl(void) {
  // class_decl -> "class" IDENTIFIER ( "<" IDENTIFIER )? "{" fun_decl* "}" ;

  Token name = consume(TokenKind::TK_IDENTIFIER, "expect class name");
  ExprPtr superclass;
  if (match({TokenKind::TK_LT})) {
    consume(TokenKind::TK_IDENTIFIER, "expect superclass name");
    superclass = std::make_shared<VariableExpr>(prev_);
  }
  consume(TokenKind::TK_LBRACE, "expect `{` before class body");
  std::vector<FunctionStmtPtr> methods;
  while (!is_end() && !check(TokenKind::TK_RBRACE)) {
    auto method = std::static_pointer_cast<FunctionStmt>(fun_decl("method"));
    methods.push_back(method);
  }
  consume(TokenKind::TK_RBRACE, "expect `}` after class body");

  return std::make_shared<ClassStmt>(name, superclass, methods);
}

StmtPtr Parser::fun_decl(const str_t& kind) {
  // fun_decl   -> "fun" IDENTIFIER "(" parameters? ")" block_stmt ;
  // parameters -> IDENTIFIER ( "," IDENTIFIER )* ;

  Token name = consume(TokenKind::TK_IDENTIFIER, "expect " + kind + " name");
  consume(TokenKind::TK_LPAREN, "expect `(` after " + kind + " name");
  std::vector<Token> params;
  if (!check(TokenKind::TK_RPAREN)) {
    do {
      if (params.size() >= kMaxArguments) {
        throw RuntimeError(curr_,
            "cannot have more than " +
            std::to_string(kMaxArguments) + " parameters");
      }
      params.push_back(
          consume(TokenKind::TK_IDENTIFIER, "expect parameter name"));
    } while (match({TokenKind::TK_COMMA}));
  }
  consume(TokenKind::TK_RPAREN, "expect `)` after " + kind + " parameters");
  consume(TokenKind::TK_LBRACE, "expect `{` before " + kind + " body");
  auto body = block_stmt();

  return std::make_shared<FunctionStmt>(name, params, body);
}

StmtPtr Parser::var_decl(void) {
  // var_decl -> "var" IDENTIFIER ( "=" expression? ) ";" ;

  Token name = consume(TokenKind::TK_IDENTIFIER, "expect variable name");
  ExprPtr expr;
  if (match({TokenKind::TK_EQ}))
    expr = expression();
  consume(TokenKind::TK_SEMI, "expect `;` after variable declaration");

  return std::make_shared<VarStmt>(name, expr);
}

StmtPtr Parser::statement(void) {
  // statement -> expr_stmt | for_stmt | if_stmt | print_stmt
  //            | return_stmt | while_stmt | block_stmt ;

  if (match({TokenKind::KW_FOR}))
    return for_stmt();
  if (match({TokenKind::KW_IF}))
    return if_stmt();
  if (match({TokenKind::KW_PRINT}))
    return print_stmt();
  if (match({TokenKind::KW_RETURN}))
    return return_stmt();
  if (match({TokenKind::KW_WHILE}))
    return while_stmt();
  if (match({TokenKind::TK_LBRACE}))
    return std::make_shared<BlockStmt>(block_stmt());
  return expr_stmt();
}

StmtPtr Parser::expr_stmt(void) {
  // expr_stmt -> expression ";" ;

  ExprPtr expr = expression();
  consume(TokenKind::TK_SEMI, "expect `;` after expression");

  return std::make_shared<ExprStmt>(expr);
}

StmtPtr Parser::for_stmt(void) {
  // for_stmt -> "for" "(" ( var_decl | expr_stmt | ";" )
  //                       expression? ";" expression? ")" statement ;

  consume(TokenKind::TK_LPAREN, "expect `(` after keyword `for`");
  StmtPtr init_clause;
  if (match({TokenKind::TK_SEMI}))
    init_clause = nullptr;
  else if (match({TokenKind::KW_VAR}))
    init_clause = var_decl();
  else
    init_clause = expr_stmt();
  ExprPtr cond_expr;
  if (!check(TokenKind::TK_SEMI))
    cond_expr = expression();
  consume(TokenKind::TK_SEMI, "expect `;` after `for` loop condition");
  ExprPtr iter_expr;
  if (!check(TokenKind::TK_RPAREN))
    iter_expr = expression();
  consume(TokenKind::TK_RPAREN, "expect `)` after `for` loop clauses");

  auto body = statement();
  if (iter_expr) {
    body = std::make_shared<BlockStmt>(
        std::vector<StmtPtr>{body, std::make_shared<ExprStmt>(iter_expr)});
  }
  if (!cond_expr)
    cond_expr = std::make_shared<LiteralExpr>(true);
  body = std::make_shared<WhileStmt>(cond_expr, body);
  if (init_clause)
    body = std::make_shared<BlockStmt>(std::vector<StmtPtr>{init_clause, body});

  return body;
}

StmtPtr Parser::if_stmt(void) {
  // if_stmt -> "if" "(" expression ")" statement ( "else" statement )? ;

  consume(TokenKind::TK_LPAREN, "expect `(` after keyword `if`");
  ExprPtr cond = expression();
  consume(TokenKind::TK_RPAREN, "expect `)` after if condition");

  StmtPtr then_branch = statement();
  StmtPtr else_branch;
  if (match({TokenKind::KW_ELSE}))
    else_branch = statement();

  return std::make_shared<IfStmt>(cond, then_branch, else_branch);
}

StmtPtr Parser::print_stmt(void) {
  // print_stmt -> "print" ( expression ( "," expression )* )? ";" ;

  std::vector<ExprPtr> exprs;
  if (!match({TokenKind::TK_SEMI})) {
    do {
      exprs.push_back(expression());
    } while (match({TokenKind::TK_COMMA}));
    consume(TokenKind::TK_SEMI, "expect `;` after print expressions");
  }

  return std::make_shared<PrintStmt>(exprs);
}

StmtPtr Parser::return_stmt(void) {
  // return_stmt -> "return" expression? ";" ;

  Token keyword = prev_;
  ExprPtr value;
  if (!check(TokenKind::TK_SEMI))
    value = expression();
  consume(TokenKind::TK_SEMI, "expect `;` after return value");

  return std::make_shared<ReturnStmt>(keyword, value);
}

StmtPtr Parser::while_stmt(void) {
  // while_stmt -> "while" "(" expression ")" statement ;

  consume(TokenKind::TK_LPAREN, "expect `(` after keyword `while`");
  ExprPtr cond = expression();
  consume(TokenKind::TK_RPAREN, "expect `)` after while condition");
  StmtPtr body = statement();

  return std::make_shared<WhileStmt>(cond, body);
}

std::vector<StmtPtr> Parser::block_stmt(void) {
  // block_stmt -> "{" declaration* "}" ;

  std::vector<StmtPtr> stmts;
  while (!is_end() && !check(TokenKind::TK_RBRACE))
    stmts.push_back(declaration());
  consume(TokenKind::TK_RBRACE, "expect `}` after block");

  return stmts;
}

ExprPtr Parser::expression(void) {
  // expression -> assignment ;

  return assignment();
}

ExprPtr Parser::assignment(void) {
  // assignment -> ( call "." )? IDENTIFIER "=" assignment | logical_or ;

  ExprPtr expr = logical_or();
  if (match({TokenKind::TK_EQ})) {
    const Token& oper = prev_;
    ExprPtr value = assignment();

    if (std::dynamic_pointer_cast<VariableExpr>(expr)) {
      const Token& name = std::static_pointer_cast<VariableExpr>(expr)->name();
      return std::make_shared<AssignExpr>(name, oper, value);
    }
    else if (std::dynamic_pointer_cast<GetExpr>(expr)) {
      GetExprPtr get = std::static_pointer_cast<GetExpr>(expr);
      return std::make_shared<SetExpr>(get->object(), get->name(), value);
    }
    throw RuntimeError(oper, "invalid assignment target");
  }

  return expr;
}

ExprPtr Parser::logical_or(void) {
  // logical_or -> logical_and ( "or" logical_and )* ;

  ExprPtr expr = logical_and();
  while (match({TokenKind::KW_OR})) {
    const Token& oper = prev_;
    ExprPtr right = logical_and();
    expr = std::make_shared<LogicalExpr>(expr, oper, right);
  }

  return expr;
}

ExprPtr Parser::logical_and(void) {
  // logical_and -> equality ( "and" equality )* ;

  ExprPtr expr = equality();
  while (match({TokenKind::KW_AND})) {
    const Token& oper = prev_;
    ExprPtr right = equality();
    expr = std::make_shared<LogicalExpr>(expr, oper, right);
  }

  return expr;
}

ExprPtr Parser::equality(void) {
  return nullptr;
}

ExprPtr Parser::comparison(void) {
  return nullptr;
}

ExprPtr Parser::addition(void) {
  return nullptr;
}

ExprPtr Parser::multiplication(void) {
  return nullptr;
}

ExprPtr Parser::unary(void) {
  return nullptr;
}

ExprPtr Parser::call(void) {
  return nullptr;
}

ExprPtr Parser::primary(void) {
  return nullptr;
}

}
