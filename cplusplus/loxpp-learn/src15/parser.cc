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
  while (!is_end()) {
    if (!ignore_newlines())
      stmts.push_back(declaration());
  }
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

bool Parser::ignore_newlines(void) {
  if (!match({TokenKind::TK_NEWLINE}))
    return false;

  while (match({TokenKind::TK_NEWLINE})) {
  }
  return true;
}

void Parser::synchronize(void) {
  advance();
  while (!is_end()) {
    if(prev().get_kind() == TokenKind::TK_NEWLINE)
      return;

    switch (prev().get_kind()) {
    case TokenKind::KW_IF:
    case TokenKind::KW_FOR:
    case TokenKind::KW_WHILE:
    case TokenKind::KW_BREAK:
    case TokenKind::KW_RETURN:
    case TokenKind::KW_FN:
    case TokenKind::KW_CLASS:
    case TokenKind::KW_LET:
    case TokenKind::KW_IMPORT:
    case TokenKind::KW_PRINT:
      return;
    }
    advance();
  }
}

ExprPtr Parser::expression(void) {
  // expression -> equality ;

  return assignment();
}

ExprPtr Parser::assignment(void) {
  // assignment   -> ( call "." )? IDENTIFILER ( assign_oper ) assignment | logical_or ;
  // assign_oper  -> "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;

  ExprPtr expr = logical_or();
  if (match({TokenKind::TK_EQUAL, TokenKind::TK_PLUSEQUAL,
        TokenKind::TK_MINUSEQUAL, TokenKind::TK_STAREQUAL,
        TokenKind::TK_SLASHEQUAL, TokenKind::TK_PERCENTEQUAL})) {
    const Token& oper = prev();
    ExprPtr value = assignment();
    if (std::dynamic_pointer_cast<VariableExpr>(expr)) {
      const Token& name = std::static_pointer_cast<VariableExpr>(expr)->name();
      return std::make_shared<AssignExpr>(name, oper, value);
    }
    else if (std::dynamic_pointer_cast<GetExpr>(expr)) {
      GetExprPtr get = std::static_pointer_cast<GetExpr>(expr);
      return std::make_shared<SetExpr>(get->object(), get->name(), value);
    }
    throw RuntimeError(oper, "invalid assignment target ...");
  }
  return expr;
}

ExprPtr Parser::logical_or(void) {
  // logical_or -> logical_and ( "or" logical_and )* ;

  ExprPtr expr = logical_and();
  while (match({TokenKind::KW_OR})) {
    const Token& oper = prev();
    ExprPtr right = logical_and();
    expr = std::make_shared<LogicalExpr>(expr, oper, right);
  }
  return expr;
}

ExprPtr Parser::logical_and(void) {
  // logical_and -> equality ( "and" equality )*;

  ExprPtr expr = equality();
  while (match({TokenKind::KW_AND})) {
    const Token& oper = prev();
    ExprPtr right = equality();
    expr = std::make_shared<LogicalExpr>(expr, oper, right);
  }
  return expr;
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
  // unary -> ( "-" | "!" | "not" ) unary | call ;

  if (match({TokenKind::TK_MINUS, TokenKind::TK_EXCLAIM, TokenKind::KW_NOT})) {
    const Token& oper = prev();
    ExprPtr right = unary();
    return std::make_shared<UnaryExpr>(oper, right);
  }

  return call();
}

ExprPtr Parser::call(void) {
  // call       -> primary ( "(" arguments? ")" | "." IDENTIFILER )* ;
  // arguments  -> expression ( "," expression )* ;

  auto finish_call = [this](const ExprPtr& callee) -> ExprPtr {
    std::vector<ExprPtr> arguments;
    if (!check(TokenKind::TK_RPAREN)) {
      do {
        if (arguments.size() >= kMaxArguments) {
          throw RuntimeError(peek(),
              "cannot have more than " +
              std::to_string(kMaxArguments) + " arguments ...");
        }
        arguments.push_back(expression());
      } while (match({TokenKind::TK_COMMA}));
    }
    const Token& paren = consume(
        TokenKind::TK_RPAREN, "expect `)` after function arguments ...");
    return std::make_shared<CallExpr>(callee, paren, arguments);
  };

  ExprPtr expr = primary();
  while (true) {
    if (match({TokenKind::TK_LPAREN})) {
      expr = finish_call(expr);
    }
    else if (match({TokenKind::TK_PERIOD})) {
      const Token& name = consume(TokenKind::TK_IDENTIFILER,
          "expect property name after `.` of class instance ...");
      expr = std::make_shared<GetExpr>(expr, name);
    }
    else {
      break;
    }
  }
  return expr;
}

ExprPtr Parser::primary(void) {
  // primary  -> INTEGER | DECIMAL | STRING | "true" | "false" | "nil" | "self"
  //          | "(" expression ")" | IDENTIFILER ;

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

  if (match({TokenKind::KW_SELF}))
    return std::make_shared<SelfExpr>(prev());
  if (match({TokenKind::TK_IDENTIFILER}))
    return std::make_shared<VariableExpr>(prev());

  throw RuntimeError(peek(), "expect expression ...");
}

StmtPtr Parser::declaration(void) {
  // declaration -> func_decl | let_decl | statement ;

  try {
    if (match({TokenKind::KW_CLASS}))
      return class_decl();
    if (match({TokenKind::KW_FN}))
      return func_decl("function");
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

StmtPtr Parser::class_decl(void) {
  // class_decl -> "class" IDENTIFILER ( ":" IDENTIFILER )? "{" function* "}" ;
  // function   -> IDENTIFILER "(" parameters? ")" block_stmt ;
  // parameters -> IDENTIFILER ( "," IDENTIFILER )* ;

  const Token& name = consume(
      TokenKind::TK_IDENTIFILER, "expect class name ...");
  ExprPtr superclass;
  if (match({TokenKind::TK_COLON})) {
    consume(TokenKind::TK_IDENTIFILER, "expect superclass name ...");
    superclass = std::make_shared<VariableExpr>(prev());
  }
  consume(TokenKind::TK_LBRACE, "expect `{` before class body ...");
  std::vector<FunctionStmtPtr> methods;
  ignore_newlines();
  while (!is_end() && !check(TokenKind::TK_RBRACE)) {
    auto meth = std::static_pointer_cast<FunctionStmt>(func_decl("method"));
    methods.push_back(meth);
    ignore_newlines();
  }
  consume(TokenKind::TK_RBRACE, "expect `}` after class body ...");
  return std::make_shared<ClassStmt>(name, superclass, methods);
}

StmtPtr Parser::func_decl(const std::string& kind) {
  // func_decl  -> "fn" function ;
  // function   -> IDENTIFILER "(" parameters? ")" block_stmt ;
  // parameters -> IDENTIFILER ( "," IDENTIFILER )* ;

  const Token& name = consume(
      TokenKind::TK_IDENTIFILER, "expect " + kind + " name ...");
  consume(TokenKind::TK_LPAREN, "expect `(` after " + kind + " name ...");
  std::vector<Token> parameters;
  if (!check(TokenKind::TK_RPAREN)) {
    do {
      if (parameters.size() >= kMaxArguments) {
        throw RuntimeError(peek(),
            "cannot have move than "
            + std::to_string(kMaxArguments) + " parameters ...");
      }
      parameters.push_back(
          consume(TokenKind::TK_IDENTIFILER, "expect parameter name ..."));
    } while (match({TokenKind::TK_COMMA}));
  }
  consume(TokenKind::TK_RPAREN, "expect `)` after " + kind + " parameters ...");
  consume(TokenKind::TK_LBRACE, "expect `{` before " + kind + " body ...");
  auto body = block_stmt();
  return std::make_shared<FunctionStmt>(name, parameters, body);
}

StmtPtr Parser::let_decl(void) {
  // let_decl -> "let" IDENTIFILER ( "=" expression? ) NEWLINE ;

  const Token& name = consume(TokenKind::TK_IDENTIFILER, "expect variable name ...");
  ExprPtr expr;
  if (match({TokenKind::TK_EQUAL}))
    expr = expression();
  consume(TokenKind::TK_NEWLINE, "expect `NL` after variable declaration ...");
  return std::make_shared<LetStmt>(name, expr);
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
  // expr_stmt -> expression NEWLINE ;

  ExprPtr expr = expression();
  consume(TokenKind::TK_NEWLINE, "expect `NL` after expression ...");
  return std::make_shared<ExprStmt>(expr);
}

StmtPtr Parser::for_stmt(void) {
  // for_stmt     -> "for" "(" init_clause cond_expr iter_expr ")" statement ;

  // for_stmt     -> "for" "(" init_clause expression? ";" expression? ")" statement ;
  // init_clause  -> loop_let | loop_expr | ";" ;
  // loop_let     -> "let" IDENTIFILER ( "=" expression? ) ";" ;
  // loop_expr    -> expression ";" ;

  auto loop_let = [this](void) -> StmtPtr {
    const Token& name = consume(TokenKind::TK_IDENTIFILER,
        "expect variable name in `for` variable declaration ... ");
    ExprPtr expr;
    if (match({TokenKind::TK_EQUAL}))
      expr = expression();
    consume(TokenKind::TK_SEMI, "expect `;` in `for` variable declaration ...");
    return std::make_shared<LetStmt>(name, expr);
  };
  auto loop_expr = [this](void) -> StmtPtr {
    ExprPtr expr = expression();
    consume(TokenKind::TK_SEMI, "expect `;` in `for` expression statement ...");
    return std::make_shared<ExprStmt>(expr);
  };

  // `for` loop operator parsing
  consume(TokenKind::TK_LPAREN, "expect `(` after `for` keyword ...");
  StmtPtr init_clause;
  if (match({TokenKind::TK_SEMI}))
    init_clause = nullptr;
  else if (match({TokenKind::KW_LET}))
    init_clause = loop_let();
  else
    init_clause = loop_expr();
  ExprPtr cond_expr;
  if (!check(TokenKind::TK_SEMI))
    cond_expr = expression();
  consume(TokenKind::TK_SEMI, "expect `;` after `for` loop condition ...");
  ExprPtr iter_expr;
  if (!check(TokenKind::TK_RPAREN))
    iter_expr = expression();
  consume(TokenKind::TK_RPAREN, "expect `)` after `for` loop clauses ...");
  ignore_newlines(); // skip NL before for statement
  StmtPtr body = statement();

  if (iter_expr) {
    std::vector<StmtPtr> stmts{body, std::make_shared<ExprStmt>(iter_expr)};
    body = std::make_shared<BlockStmt>(stmts);
  }
  if (!cond_expr)
    cond_expr = std::make_shared<LiteralExpr>(true);
  body = std::make_shared<WhileStmt>(cond_expr, body);
  if (init_clause) {
    std::vector<StmtPtr> stmts{init_clause, body};
    body = std::make_shared<BlockStmt>(stmts);
  }

  return body;
}

StmtPtr Parser::if_stmt(void) {
  // if_stmt -> "if" "(" expression ")" statement ( "else" statement )? ;

  consume(TokenKind::TK_LPAREN, "expect `(` after `if` keyword ...");
  ExprPtr cond = expression();
  consume(TokenKind::TK_RPAREN, "expect `)` after if condition ...");
  ignore_newlines(); // skip NL before if statement
  StmtPtr then_branch = statement();
  StmtPtr else_branch;
  if (match({TokenKind::KW_ELSE})) {
    ignore_newlines(); // skip NL before else statement
    else_branch = statement();
  }
  return std::make_shared<IfStmt>(cond, then_branch, else_branch);
}

StmtPtr Parser::print_stmt(void) {
  // print_stmt -> "print" ( expression ( "," expression )* )? NEWLINE ;

  std::vector<ExprPtr> exprs;
  if (!match({TokenKind::TK_NEWLINE})) {
    do {
      exprs.push_back(expression());
    } while (match({TokenKind::TK_COMMA}));
    consume(TokenKind::TK_NEWLINE, "expect `NL` after `print` expression ...");
  }
  return std::make_shared<PrintStmt>(exprs);
}

StmtPtr Parser::return_stmt(void) {
  // return_stmt -> "return" expression? NEWLINE ;

  const Token& keyword = prev();
  ExprPtr value;
  if (!check(TokenKind::TK_NEWLINE))
    value = expression();
  consume(TokenKind::TK_NEWLINE, "expect `NL` after return value ...");
  return std::make_shared<ReturnStmt>(keyword, value);
}

StmtPtr Parser::while_stmt(void) {
  // while_stmt -> "while" "(" expression ")" statement ;

  consume(TokenKind::TK_LPAREN, "expect `(` after `while` keyword ...");
  ExprPtr cond = expression();
  consume(TokenKind::TK_RPAREN, "expect `)` after while condition ...");
  ignore_newlines(); // skip NL before while statement
  StmtPtr body = statement();
  return std::make_shared<WhileStmt>(cond, body);
}

std::vector<StmtPtr> Parser::block_stmt(void) {
  // block_stmt -> "{" declaration* "}" ;

  std::vector<StmtPtr> stmts;
  while (!is_end() && !check(TokenKind::TK_RBRACE)) {
    if (!ignore_newlines())
      stmts.push_back(declaration());
  }
  consume(TokenKind::TK_RBRACE, "expect `}` after block statement ...");
  return stmts;
}

}
