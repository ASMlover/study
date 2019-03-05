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
#pragma once

#include <memory>
#include <vector>
#include "../common/common.hh"
#include "../common/value.hh"
#include "../lex/token.hh"

namespace sage {

struct Expr;
struct Stmt;
struct ExprVisitor;
struct StmtVisitor;

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;
using ExprVisitorPtr = std::shared_ptr<ExprVisitor>;
using StmtVisitorPtr = std::shared_ptr<StmtVisitor>;

struct Expr : private UnCopyable {
  virtual ~Expr(void) {}
  virtual void accept(const ExprVisitorPtr& visitor) = 0;
};

class AssignExpr
  : public Expr, public std::enable_shared_from_this<AssignExpr> {
  Token name_;
  Token oper_;
  ExprPtr value_;
public:
  const Token& name(void) const { return name_; }
  const Token& oper(void) const { return oper_; }
  const ExprPtr& value(void) const { return value_; }

  AssignExpr(const Token& name, const Token& oper, const ExprPtr& value)
    : name_(name), oper_(oper), value_(value) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using AssignExprPtr = std::shared_ptr<AssignExpr>;

class BinaryExpr
  : public Expr, public std::enable_shared_from_this<BinaryExpr> {
  ExprPtr left_;
  Token oper_;
  ExprPtr right_;
public:
  const ExprPtr& left(void) const { return left_; }
  const Token& oper(void) const { return oper_; }
  const ExprPtr& right(void) const { return right_; }

  BinaryExpr(const ExprPtr& left, const Token& oper, const ExprPtr& right)
    : left_(left), oper_(oper), right_(right) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using BinaryExprPtr = std::shared_ptr<BinaryExpr>;

class CallExpr
  : public Expr, public std::enable_shared_from_this<CallExpr> {
  ExprPtr callee_;
  Token paren_;
  std::vector<ExprPtr> arguments_;
public:
  const ExprPtr& callee(void) const { return callee_; }
  const Token& paren(void) const { return paren_; }
  const std::vector<ExprPtr>& arguments(void) const { return arguments_; }

  CallExpr(const ExprPtr& callee,
      const Token& paren, const std::vector<ExprPtr>& arguments)
    : callee_(callee), paren_(paren), arguments_(arguments) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using CallExprPtr = std::shared_ptr<CallExpr>;

class SetExpr
  : public Expr, public std::enable_shared_from_this<SetExpr> {
  ExprPtr object_;
  Token name_;
  ExprPtr value_;
public:
  const ExprPtr& object(void) const { return object_; }
  const Token& name(void) const { return name_; }
  const ExprPtr& value(void) const { return value_; }

  SetExpr(const ExprPtr& object, const Token& name, const ExprPtr& value)
    : object_(object), name_(name), value_(value) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using SetExprPtr = std::shared_ptr<SetExpr>;

class GetExpr
  : public Expr, public std::enable_shared_from_this<GetExpr> {
  ExprPtr object_;
  Token name_;
public:
  const ExprPtr& object(void) const { return object_; }
  const Token& name(void) const { return name_; }

  GetExpr(const ExprPtr& object, const Token& name)
    : object_(object), name_(name) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using GetExprPtr = std::shared_ptr<GetExpr>;

class GroupingExpr
  : public Expr, public std::enable_shared_from_this<GroupingExpr> {
  ExprPtr expression_;
public:
  const ExprPtr& expression(void) const { return expression_; }

  GroupingExpr(const ExprPtr& expression)
    : expression_(expression) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using GroupingExprPtr = std::shared_ptr<GroupingExpr>;

class LiteralExpr
  : public Expr, public std::enable_shared_from_this<LiteralExpr> {
  Value value_;
public:
  const Value& value(void) const { return value_; }

  LiteralExpr(const Value& value)
    : value_(value) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using LiteralExprPtr = std::shared_ptr<LiteralExpr>;

class LogicalExpr
  : public Expr, public std::enable_shared_from_this<LogicalExpr> {
  ExprPtr left_;
  Token oper_;
  ExprPtr right_;
public:
  const ExprPtr& left(void) const { return left_; }
  const Token& oper(void) const { return oper_; }
  const ExprPtr& right(void) const { return right_; }

  LogicalExpr(const ExprPtr& left, const Token& oper, const ExprPtr& right)
    : left_(left), oper_(oper), right_(right) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using LogicalExprPtr = std::shared_ptr<LogicalExpr>;

class SelfExpr
  : public Expr, public std::enable_shared_from_this<SelfExpr> {
  Token keyword_;
public:
  const Token& keyword(void) const { return keyword_; }

  SelfExpr(const Token& keyword)
    : keyword_(keyword) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using SelfExprPtr = std::shared_ptr<SelfExpr>;

class SuperExpr
  : public Expr, public std::enable_shared_from_this<SuperExpr> {
  Token keyword_;
  Token method_;
public:
  const Token& keyword(void) const { return keyword_; }
  const Token& method(void) const { return method_; }

  SuperExpr(const Token& keyword, const Token& method)
    : keyword_(keyword), method_(method) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using SuperExprPtr = std::shared_ptr<SuperExpr>;

class UnaryExpr
  : public Expr, public std::enable_shared_from_this<UnaryExpr> {
  Token oper_;
  ExprPtr right_;
public:
  const Token& oper(void) const { return oper_; }
  const ExprPtr& right(void) const { return right_; }

  UnaryExpr(const Token& oper, const ExprPtr& right)
    : oper_(oper), right_(right) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using UnaryExprPtr = std::shared_ptr<UnaryExpr>;

class VariableExpr
  : public Expr, public std::enable_shared_from_this<VariableExpr> {
  Token name_;
public:
  const Token& name(void) const { return name_; }

  VariableExpr(const Token& name)
    : name_(name) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using VariableExprPtr = std::shared_ptr<VariableExpr>;

class FunctionExpr
  : public Expr, public std::enable_shared_from_this<FunctionExpr> {
  std::vector<Token> params_;
  std::vector<StmtPtr> body_;
public:
  const std::vector<Token>& params(void) const { return params_; }
  const std::vector<StmtPtr>& body(void) const { return body_; }

  FunctionExpr(
      const std::vector<Token>& params, const std::vector<StmtPtr>& body)
    : params_(params), body_(body) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using FunctionExprPtr = std::shared_ptr<FunctionExpr>;

struct ExprVisitor : private UnCopyable {
  virtual ~ExprVisitor(void) {}

  virtual void visit(const AssignExprPtr& expr) = 0;
  virtual void visit(const BinaryExprPtr& expr) = 0;
  virtual void visit(const CallExprPtr& expr) = 0;
  virtual void visit(const SetExprPtr& expr) = 0;
  virtual void visit(const GetExprPtr& expr) = 0;
  virtual void visit(const GroupingExprPtr& expr) = 0;
  virtual void visit(const LiteralExprPtr& expr) = 0;
  virtual void visit(const LogicalExprPtr& expr) = 0;
  virtual void visit(const SelfExprPtr& expr) = 0;
  virtual void visit(const SuperExprPtr& expr) = 0;
  virtual void visit(const UnaryExprPtr& expr) = 0;
  virtual void visit(const VariableExprPtr& expr) = 0;
  virtual void visit(const FunctionExprPtr& expr) = 0;
};

// program    -> statement* EOF ;
// statement  -> expr_stmt | print_stmt ;
// expr_stmt  -> expression NL ;
// print_stmt -> "print" ( expression ( "," expression )* )? NL ;

struct Stmt : private UnCopyable {
  virtual ~Stmt(void) {}
  virtual void accept(const StmtVisitorPtr& visitor) = 0;
};

class ExprStmt
  : public Stmt, public std::enable_shared_from_this<ExprStmt> {
  ExprPtr expr_;
public:
  const ExprPtr& expr(void) const { return expr_; }

  ExprStmt(const ExprPtr& expr)
    : expr_(expr) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using ExprStmtPtr = std::shared_ptr<ExprStmt>;

class PrintStmt
  : public Stmt, public std::enable_shared_from_this<PrintStmt> {
  std::vector<ExprPtr> exprs_;
public:
  const std::vector<ExprPtr>& exprs(void) const { return exprs_; }

  PrintStmt(const std::vector<ExprPtr>& exprs)
    : exprs_(exprs) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using PrintStmtPtr = std::shared_ptr<PrintStmt>;

class LetStmt
  : public Stmt, public std::enable_shared_from_this<LetStmt> {
  Token name_;
  ExprPtr expr_;
public:
  const Token& name(void) const { return name_; }
  const ExprPtr& expr(void) const { return expr_; }

  LetStmt(const Token& name, const ExprPtr& expr)
    : name_(name), expr_(expr) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using LetStmtPtr = std::shared_ptr<LetStmt>;

class BlockStmt
  : public Stmt, public std::enable_shared_from_this<BlockStmt> {
  std::vector<StmtPtr> stmts_;
public:
  const std::vector<StmtPtr>& stmts(void) const { return stmts_; }

  BlockStmt(const std::vector<StmtPtr>& stmts)
    : stmts_(stmts) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using BlockStmtPtr = std::shared_ptr<BlockStmt>;

class IfStmt
  : public Stmt, public std::enable_shared_from_this<IfStmt> {
  ExprPtr cond_;
  std::vector<StmtPtr> then_branch_;
  std::vector<StmtPtr> else_branch_;
public:
  const ExprPtr& cond(void) const { return cond_; }
  const std::vector<StmtPtr>& then_branch(void) const { return then_branch_; }
  const std::vector<StmtPtr>& else_branch(void) const { return else_branch_; }

  IfStmt(const ExprPtr& cond,
      const std::vector<StmtPtr>& then_branch,
      const std::vector<StmtPtr>& else_branch)
    : cond_(cond), then_branch_(then_branch), else_branch_(else_branch) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using IfStmtPtr = std::shared_ptr<IfStmt>;

class WhileStmt
  : public Stmt, public std::enable_shared_from_this<WhileStmt> {
  ExprPtr cond_;
  std::vector<StmtPtr> body_;
public:
  const ExprPtr& cond(void) const { return cond_; }
  const std::vector<StmtPtr>& body(void) const { return body_; }

  WhileStmt(const ExprPtr& cond, const std::vector<StmtPtr>& body)
    : cond_(cond), body_(body) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using WhileStmtPtr = std::shared_ptr<WhileStmt>;

class FunctionStmt
  : public Stmt, public std::enable_shared_from_this<FunctionStmt> {
  Token name_;
  std::vector<Token> params_;
  std::vector<StmtPtr> body_;
public:
  const Token& name(void) const { return name_; }
  const std::vector<Token>& params(void) const { return params_; }
  const std::vector<StmtPtr>& body(void) const { return body_; }

  FunctionStmt(const Token& name,
      const std::vector<Token>& params, const std::vector<StmtPtr>& body)
    : name_(name), params_(params), body_(body) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using FunctionStmtPtr = std::shared_ptr<FunctionStmt>;

class ReturnStmt
  : public Stmt, public std::enable_shared_from_this<ReturnStmt> {
  Token keyword_;
  ExprPtr value_;
public:
  const Token& keyword(void) const { return keyword_; }
  const ExprPtr& value(void) const { return value_; }

  ReturnStmt(const Token& keyword, const ExprPtr& value)
    : keyword_(keyword), value_(value) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using ReturnStmtPtr = std::shared_ptr<ReturnStmt>;

class BreakStmt
  : public Stmt, public std::enable_shared_from_this<BreakStmt> {
  Token keyword_;
public:
  const Token& keyword(void) const { return keyword_; }

  BreakStmt(const Token& keyword)
    : keyword_(keyword) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using BreakStmtPtr = std::shared_ptr<BreakStmt>;

class ClassStmt
  : public Stmt, public std::enable_shared_from_this<ClassStmt> {
  Token name_;
  ExprPtr superclass_;
  std::vector<FunctionStmtPtr> methods_;
public:
  const Token& name(void) const { return name_; }
  const ExprPtr& superclass(void) const { return superclass_; }
  const std::vector<FunctionStmtPtr>& methods(void) const { return methods_; }

  ClassStmt(const Token& name,
      const ExprPtr& superclass, const std::vector<FunctionStmtPtr>& methods)
    : name_(name), superclass_(superclass), methods_(methods) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};
using ClassStmtPtr = std::shared_ptr<ClassStmt>;

struct StmtVisitor : private UnCopyable {
  virtual ~StmtVisitor(void) {}

  virtual void visit(const ExprStmtPtr& stmt) = 0;
  virtual void visit(const PrintStmtPtr& stmt) = 0;
  virtual void visit(const LetStmtPtr& stmt) = 0;
  virtual void visit(const BlockStmtPtr& stmt) = 0;
  virtual void visit(const IfStmtPtr& stmt) = 0;
  virtual void visit(const WhileStmtPtr& stmt) = 0;
  virtual void visit(const FunctionStmtPtr& stmt) = 0;
  virtual void visit(const ReturnStmtPtr& stmt) = 0;
  virtual void visit(const BreakStmtPtr& stmt) = 0;
  virtual void visit(const ClassStmtPtr& stmt) = 0;
};

}
