// Copyright (c) 2025 ASMlover. All rights reserved.
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

#include <memory>
#include <vector>
#include <format>
#include "Common.hh"
#include "Token.hh"
#include "Value.hh"

namespace ms {

struct Expr;
struct Stmt;
class Interpreter;
class VariableExpr;
class FunctionStmt;

using ExprPtr         = std::shared_ptr<Expr>;
using StmtPtr         = std::shared_ptr<Stmt>;
using VariableExprPtr = std::shared_ptr<VariableExpr>;
using FunctionStmtPtr = std::shared_ptr<FunctionStmt>;

struct Expr : private UnCopyable {
  virtual ~Expr() noexcept = default;
  virtual Value accept(Interpreter& interpreter) const = 0;
};

struct Stmt : private UnCopyable {
  virtual ~Stmt() noexcept = default;
  virtual void accept(Interpreter& interpreter) const = 0;
};

// -------------------------------- Expression ---------------------------------
class LiteralExpr final : public Expr {
  Value value_;
public:
  LiteralExpr(Value value) noexcept : value_{std::move(value)} {}
  virtual Value accept(Interpreter& interpreter) const override;

  inline const Value& value() const noexcept { return value_; }
};

class GroupingExpr final : public Expr {
  ExprPtr expression_;
public:
  GroupingExpr(ExprPtr expression) noexcept : expression_{std::move(expression)} {}
  virtual Value accept(Interpreter& interpreter) const override;

  inline const ExprPtr& expression() const noexcept { return expression_; }
};

class UnaryExpr final : public Expr {
  Token op_;
  ExprPtr right_;
public:
  UnaryExpr(const Token& op, ExprPtr right) noexcept : op_{op}, right_{std::move(right)} {}
  virtual Value accept(Interpreter& interpreter) const override;

  inline const Token& op() const noexcept { return op_; }
  inline const ExprPtr& right() const noexcept { return right_; }
};

class BinayExpr final : public Expr {
  ExprPtr left_;
  Token op_;
  ExprPtr right_;
public:
  BinayExpr(ExprPtr left, const Token& op, ExprPtr right) noexcept
    : left_{std::move(left)}, op_{op}, right_{std::move(right)} {
  }
  virtual Value accept(Interpreter& interpreter) const override;

  inline const ExprPtr& left() const noexcept { return left_; }
  inline const Token& op() const noexcept { return op_; }
  inline const ExprPtr& right() const noexcept { return right_; }
};

class LogicalExpr final : public Expr {
  ExprPtr left_;
  Token op_;
  ExprPtr right_;
public:
  LogicalExpr(ExprPtr left, const Token& op, ExprPtr right) noexcept
    : left_{std::move(left)}, op_{op}, right_{std::move(right)} {
  }
  virtual Value accept(Interpreter& interpreter) const override;

  inline const ExprPtr& left() const noexcept { return left_; }
  inline const Token& op() const noexcept { return op_; }
  inline const ExprPtr& right() const noexcept { return right_; }
};

class VariableExpr final : public Expr {
  Token name_;
public:
  VariableExpr(const Token& name) noexcept : name_{name} {}
  virtual Value accept(Interpreter& interpreter) const override;

  inline const Token& name() const noexcept { return name_; }
};

class AssignExpr final : public Expr {
  Token name_;
  ExprPtr value_;
public:
  AssignExpr(const Token& name, ExprPtr value) noexcept : name_{name}, value_{std::move(value)} {}
  virtual Value accept(Interpreter& interpreter) const override;

  inline const Token& name() const noexcept { return name_; }
  inline const ExprPtr& value() const noexcept { return value_; }
};

class CallExpr final : public Expr {
  ExprPtr callee_;
  Token paren_;
  std::vector<ExprPtr> arguments_;
public:
  CallExpr(ExprPtr callee, const Token& paren, const std::vector<ExprPtr>& arguments) noexcept
    : callee_{std::move(callee)}, paren_{paren}, arguments_{arguments} {
  }
  virtual Value accept(Interpreter& interpreter) const override;

  inline const ExprPtr& callee() const noexcept { return callee_; }
  inline const Token& paren() const noexcept { return paren_; }
  inline const std::vector<ExprPtr>& arguments() const noexcept { return arguments_; }
};

class GetExpr final : public Expr {
  ExprPtr object_;
  Token name_;
public:
  GetExpr(ExprPtr object, const Token& name) noexcept : object_{std::move(object)}, name_{name} {}
  virtual Value accept(Interpreter& interpreter) const override;

  inline const ExprPtr& object() const noexcept { return object_; }
  inline const Token& name() const noexcept { return name_; }
};

class SetExpr final : public Expr {
  ExprPtr object_;
  Token name_;
  ExprPtr value_;
public:
  SetExpr(ExprPtr object, const Token& name, ExprPtr value) noexcept
    : object_{std::move(object)}, name_{name}, value_{std::move(value)} {
  }
  virtual Value accept(Interpreter& interpreter) const override;

  inline const ExprPtr& object() const noexcept { return object_; }
  inline const Token& name() const noexcept { return name_; }
  inline const ExprPtr& value() const noexcept { return value_; }
};

class ThisExpr final : public Expr {
  Token keyword_;
public:
  ThisExpr(const Token& keyword) noexcept : keyword_{keyword} {}
  virtual Value accept(Interpreter& interpreter) const override;

  inline const Token& keyword() const noexcept { return keyword_; }
};

class SuperExpr final : public Expr {
  Token keyword_;
  Token method_;
public:
  SuperExpr(const Token& keyword, const Token& method) noexcept : keyword_{keyword}, method_{method} {}
  virtual Value accept(Interpreter& interpreter) const override;

  inline const Token& keyword() const noexcept { return keyword_; }
  inline const Token& method() const noexcept { return method_; }
};

// --------------------------------- Statement ---------------------------------
class ExpressionStmt final : public Stmt {
  ExprPtr expression_;
public:
  ExpressionStmt(ExprPtr expression) noexcept : expression_{std::move(expression)} {}
  virtual void accept(Interpreter& interpreter) const override;

  inline const ExprPtr& expression() const noexcept { return expression_; }
};

class PrintStmt final : public Stmt {
  ExprPtr expression_;
public:
  PrintStmt(ExprPtr expression) noexcept : expression_{std::move(expression)} {}
  virtual void accept(Interpreter& interpreter) const override;

  inline const ExprPtr& expression() const noexcept { return expression_; }
};

class VarStmt final : public Stmt {
  Token name_;
  ExprPtr initializer_;
public:
  VarStmt(const Token& name, ExprPtr initializer) noexcept : name_{name}, initializer_{std::move(initializer)} {}
  virtual void accept(Interpreter& interpreter) const override;

  inline const Token& name() const noexcept { return name_; }
  inline const ExprPtr& initializer() const noexcept { return initializer_; }
};

class BlockStmt final : public Stmt {
  std::vector<StmtPtr> statements_;
public:
  BlockStmt(const std::vector<StmtPtr>& statements) noexcept : statements_{statements} {}
  virtual void accept(Interpreter& interpreter) const override;

  inline const std::vector<StmtPtr>& statements() const noexcept { return statements_; }
};

class IfStmt final : public Stmt {
  ExprPtr condition_;
  StmtPtr then_branch_;
  StmtPtr else_branch_;
public:
  IfStmt(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch) noexcept
    : condition_{std::move(condition)}, then_branch_{std::move(then_branch)}, else_branch_{std::move(else_branch)} {
  }
  virtual void accept(Interpreter& interpreter) const override;

  inline const ExprPtr& condition() const noexcept { return condition_; }
  inline const StmtPtr& then_branch() const noexcept { return then_branch_; }
  inline const StmtPtr& else_branch() const noexcept { return else_branch_; }
};

class WhileStmt final : public Stmt {
  ExprPtr condition_;
  StmtPtr body_;
public:
  WhileStmt(ExprPtr condition, StmtPtr body) noexcept
    : condition_{std::move(condition)}, body_{std::move(body)} {
  }
  virtual void accept(Interpreter& interpreter) const override;

  inline const ExprPtr& condition() const noexcept { return condition_; }
  inline const StmtPtr& body() const noexcept { return body_; }
};

class FunctionStmt final : public Stmt {
  Token name_;
  std::vector<Token> params_;
  std::vector<StmtPtr> body_;
public:
  FunctionStmt(const Token& name, const std::vector<Token>& params, const std::vector<StmtPtr>& body) noexcept
    : name_{name}, params_{params}, body_{body} {
  }
  virtual void accept(Interpreter& interpreter) const override;

  inline const Token& name() const noexcept { return name_; }
  inline const std::vector<Token>& params() const noexcept { return params_; }
  inline const std::vector<StmtPtr>& body() const noexcept { return body_; }
};

class ClassStmt final : public Stmt {
  Token name_;
  VariableExprPtr superclass_;
  std::vector<FunctionStmtPtr> methods_;
public:
  ClassStmt(const Token& name, VariableExprPtr superclass, const std::vector<FunctionStmtPtr>& methods) noexcept
    : name_{name}, superclass_{std::move(superclass)}, methods_{methods} {
  }
  virtual void accept(Interpreter& interpreter) const override;

  inline const Token& name() const noexcept { return name_; }
  inline const VariableExprPtr& superclass() const noexcept { return superclass_; }
  inline const std::vector<FunctionStmtPtr>& methods() const noexcept { return methods_; }
};

class ImportStmt final : public Stmt {
  Token module_path_;
public:
  ImportStmt(const Token& module_path) noexcept : module_path_{module_path} {}
  virtual void accept(Interpreter& interpreter) const override;

  inline const Token& module_path() const noexcept { return module_path_; }
};

// ----------------------------------- Parser ----------------------------------
class Parser final : private UnCopyable {
  std::vector<Token> tokens_;
  sz_t current_{};

  inline bool is_at_end() const noexcept { return peek().type() == TokenType::TK_EOF; }
  inline const Token& peek() const noexcept { return tokens_[current_]; }
  inline const Token& prev() const noexcept { return tokens_[current_ - 1]; }
  inline bool check(TokenType type) const noexcept { return is_at_end() ? false : peek().type() == type; }

  Token advance() noexcept {
    if (!is_at_end())
      ++current_;
    return prev();
  }

  bool match(std::initializer_list<TokenType> types) noexcept {
    for (auto type: types) {
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

    throw std::runtime_error(std::format("Error at line {}: {}", peek().lineno(), message));
  }

  void synchronize() noexcept {
    advance();

    while (!is_at_end()) {
      if (prev().type() == TokenType::TK_SEMICOLON)
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
      default: advance();
      }
    }
  }

  StmtPtr declaration() noexcept;
  StmtPtr class_declaration() noexcept;
  FunctionStmtPtr function(const str_t& kind);
  StmtPtr var_declaration() noexcept;
  StmtPtr statement() noexcept;

  StmtPtr for_statement() noexcept;
  StmtPtr if_statement() noexcept;
  StmtPtr print_statement() noexcept;
  StmtPtr return_statement() noexcept;
  StmtPtr while_statement() noexcept;
  std::vector<StmtPtr> block() noexcept;
  StmtPtr import_statement() noexcept;
  StmtPtr expression_statement() noexcept;
  ExprPtr expression() noexcept;

  ExprPtr assignment();
  ExprPtr or_expr() noexcept;
  ExprPtr and_expr() noexcept;
  ExprPtr equality() noexcept;
  ExprPtr comparison() noexcept;
  ExprPtr term() noexcept;
  ExprPtr factor() noexcept;
  ExprPtr unary() noexcept;
  ExprPtr call() noexcept;
  ExprPtr finish_call(ExprPtr callee);
  ExprPtr primary();
public:
  Parser(const std::vector<Token>& tokens) noexcept : tokens_{tokens} {}

  inline std::vector<StmtPtr> parse() noexcept {
    std::vector<StmtPtr> statements;
    while (!is_at_end())
      statements.push_back(declaration());
    return statements;
  }
};

}
