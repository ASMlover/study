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
#include "common.hh"
#include "token.hh"
#include "interpret_value.hh"

namespace loxcc::interpret {

struct Expr;
struct Stmt;
struct ExprVisitor;
struct StmtVisitor;

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;
using ExprVisitorPtr = std::shared_ptr<ExprVisitor>;
using StmtVisitorPtr = std::shared_ptr<StmtVisitor>;

class AssignExpr;
class SetExpr;
class LogicalExpr;
class BinaryExpr;
class UnaryExpr;
class CallExpr;
class GetExpr;
class LiteralExpr;
class GroupingExpr;
class SuperExpr;
class ThisExpr;
class VariableExpr;
class FunctionExpr;

class ClassStmt;
class FunctionStmt;
class VarStmt;
class BlockStmt;
class ExprStmt;
class WhileStmt;
class IfStmt;
class PrintStmt;
class ReturnStmt;

using AssignExprPtr = std::shared_ptr<AssignExpr>;
using SetExprPtr = std::shared_ptr<SetExpr>;
using LogicalExprPtr = std::shared_ptr<LogicalExpr>;
using BinaryExprPtr = std::shared_ptr<BinaryExpr>;
using UnaryExprPtr = std::shared_ptr<UnaryExpr>;
using CallExprPtr = std::shared_ptr<CallExpr>;
using GetExprPtr = std::shared_ptr<GetExpr>;
using LiteralExprPtr = std::shared_ptr<LiteralExpr>;
using GroupingExprPtr = std::shared_ptr<GroupingExpr>;
using SuperExprPtr = std::shared_ptr<SuperExpr>;
using ThisExprPtr = std::shared_ptr<ThisExpr>;
using VariableExprPtr = std::shared_ptr<VariableExpr>;
using FunctionExprPtr = std::shared_ptr<FunctionExpr>;

using ClassStmtPtr = std::shared_ptr<ClassStmt>;
using FunctionStmtPtr = std::shared_ptr<FunctionStmt>;
using VarStmtPtr = std::shared_ptr<VarStmt>;
using BlockStmtPtr = std::shared_ptr<BlockStmt>;
using ExprStmtPtr = std::shared_ptr<ExprStmt>;
using WhileStmtPtr = std::shared_ptr<WhileStmt>;
using IfStmtPtr = std::shared_ptr<IfStmt>;
using PrintStmtPtr = std::shared_ptr<PrintStmt>;
using ReturnStmtPtr = std::shared_ptr<ReturnStmt>;

struct ExprVisitor : private UnCopyable {
  virtual ~ExprVisitor(void) {}

  virtual void visit(const AssignExprPtr& expr) = 0;
  virtual void visit(const SetExprPtr& expr) = 0;
  virtual void visit(const LogicalExprPtr& expr) = 0;
  virtual void visit(const BinaryExprPtr& expr) = 0;
  virtual void visit(const UnaryExprPtr& expr) = 0;
  virtual void visit(const CallExprPtr& expr) = 0;
  virtual void visit(const GetExprPtr& expr) = 0;
  virtual void visit(const LiteralExprPtr& expr) = 0;
  virtual void visit(const GroupingExprPtr& expr) = 0;
  virtual void visit(const SuperExprPtr& expr) = 0;
  virtual void visit(const ThisExprPtr& expr) = 0;
  virtual void visit(const VariableExprPtr& expr) = 0;
  virtual void visit(const FunctionExprPtr& expr) = 0;
};

struct StmtVisitor : private UnCopyable {
  virtual ~StmtVisitor(void) {}

  virtual void visit(const ClassStmtPtr& stmt) = 0;
  virtual void visit(const FunctionStmtPtr& stmt) = 0;
  virtual void visit(const VarStmtPtr& stmt) = 0;
  virtual void visit(const BlockStmtPtr& stmt) = 0;
  virtual void visit(const ExprStmtPtr& stmt) = 0;
  virtual void visit(const WhileStmtPtr& stmt) = 0;
  virtual void visit(const IfStmtPtr& stmt) = 0;
  virtual void visit(const PrintStmtPtr& stmt) = 0;
  virtual void visit(const ReturnStmtPtr& stmt) = 0;
};

struct Expr : private UnCopyable {
  virtual ~Expr(void) {}
  virtual void accept(const ExprVisitorPtr& visitor) = 0;
};

struct Stmt : private UnCopyable {
  virtual ~Stmt(void) {}
  virtual void accept(const StmtVisitorPtr& visitor) = 0;
};

class AssignExpr final
  : public Expr, public std::enable_shared_from_this<AssignExpr> {
  Token name_;
  Token oper_;
  ExprPtr value_;
public:
  inline const Token& name(void) const { return name_; }
  inline const Token& oper(void) const { return oper_; }
  inline const ExprPtr& value(void) const { return value_; }

  AssignExpr(
      const Token& name, const Token& oper, const ExprPtr& value) noexcept
    : name_(name), oper_(oper), value_(value) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class SetExpr final
  : public Expr, public std::enable_shared_from_this<SetExpr> {
  ExprPtr object_;
  Token name_;
  ExprPtr value_;
public:
  inline const ExprPtr& object(void) const { return object_; }
  inline const Token& name(void) const { return name_; }
  inline const ExprPtr& value(void) const { return value_; }

  SetExpr(const ExprPtr& object,
      const Token& name, const ExprPtr& value) noexcept
    : object_(object), name_(name), value_(value) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class LogicalExpr final
  : public Expr, public std::enable_shared_from_this<LogicalExpr> {
  ExprPtr lhs_;
  Token oper_;
  ExprPtr rhs_;
public:
  inline const ExprPtr& lhs(void) const { return lhs_; }
  inline const Token& oper(void) const { return oper_; }
  inline const ExprPtr& rhs(void) const { return rhs_; }

  LogicalExpr(
      const ExprPtr& lhs, const Token& oper, const ExprPtr& rhs) noexcept
    : lhs_(lhs), oper_(oper), rhs_(rhs) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class BinaryExpr final
  : public Expr, public std::enable_shared_from_this<BinaryExpr> {
  ExprPtr lhs_;
  Token oper_;
  ExprPtr rhs_;
public:
  inline const ExprPtr& lhs(void) const { return lhs_; }
  inline const Token& oper(void) const { return oper_; }
  inline const ExprPtr& rhs(void) const { return rhs_; }

  BinaryExpr(
      const ExprPtr& lhs, const Token& oper, const ExprPtr& rhs) noexcept
    : lhs_(lhs), oper_(oper), rhs_(rhs) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class UnaryExpr final
  : public Expr, public std::enable_shared_from_this<UnaryExpr> {
  Token oper_;
  ExprPtr rhs_;
public:
  inline const Token& oper(void) const { return oper_; }
  inline const ExprPtr& rhs(void) const { return rhs_; }

  UnaryExpr(const Token& oper, const ExprPtr& rhs) noexcept
    : oper_(oper), rhs_(rhs) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class CallExpr final
  : public Expr, public std::enable_shared_from_this<CallExpr> {
  ExprPtr callee_;
  Token paren_;
  std::vector<ExprPtr> arguments_;
public:
  inline const ExprPtr& callee(void) const { return callee_; }
  inline const Token& paren(void) const { return paren_; }
  inline const std::vector<ExprPtr>& arguments(void) const { return arguments_; }

  CallExpr(const ExprPtr& callee,
      const Token& paren, const std::vector<ExprPtr>& arguments) noexcept
    : callee_(callee), paren_(paren), arguments_(arguments) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class GetExpr final
  : public Expr, public std::enable_shared_from_this<GetExpr> {
  ExprPtr object_;
  Token name_;
public:
  inline const ExprPtr& object(void) const { return object_; }
  inline const Token& name(void) const { return name_; }

  GetExpr(const ExprPtr& object, const Token& name) noexcept
    : object_(object), name_(name) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class LiteralExpr final
  : public Expr, public std::enable_shared_from_this<LiteralExpr> {
  Value value_;
public:
  inline const Value& value(void) const { return value_; }

  LiteralExpr(const Value& value) noexcept
    : value_(value) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class GroupingExpr final
  : public Expr, public std::enable_shared_from_this<GroupingExpr> {
  ExprPtr expression_;
public:
  inline const ExprPtr& expression(void) const { return expression_; }

  GroupingExpr(const ExprPtr& expression) noexcept
    : expression_(expression) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class SuperExpr final
  : public Expr, public std::enable_shared_from_this<SuperExpr> {
  Token keyword_;
  Token method_;
public:
  inline const Token& keyword(void) const { return keyword_; }
  inline const Token& method(void) const { return method_; }

  SuperExpr(const Token& keyword, const Token& method) noexcept
    : keyword_(keyword), method_(method) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class ThisExpr final
  : public Expr, public std::enable_shared_from_this<ThisExpr> {
  Token keyword_;
public:
  inline const Token& keyword(void) const { return keyword_; }

  ThisExpr(const Token& keyword) noexcept
    : keyword_(keyword) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class VariableExpr final
  : public Expr, public std::enable_shared_from_this<VariableExpr> {
  Token name_;
public:
  inline const Token& name(void) const { return name_; }

  VariableExpr(const Token& name) noexcept
    : name_(name) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class FunctionExpr final
  : public Expr, public std::enable_shared_from_this<FunctionExpr> {
  std::vector<Token> params_;
  std::vector<StmtPtr> body_;
public:
  inline const std::vector<Token>& params(void) const { return params_; }
  inline const std::vector<StmtPtr>& body(void) const { return body_; }

  FunctionExpr(const std::vector<Token>& params,
      const std::vector<StmtPtr>& body) noexcept
    : params_(params), body_(body) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};

class ClassStmt final
  : public Stmt, public std::enable_shared_from_this<ClassStmt> {
  Token name_;
  ExprPtr superclass_;
  std::vector<FunctionStmtPtr> methods_;
public:
  inline const Token& name(void) const { return name_; }
  inline const ExprPtr& superclass(void) const { return superclass_; }
  inline const std::vector<FunctionStmtPtr>& methods(void) const { return methods_; }

  ClassStmt(const Token& name,
      const ExprPtr& superclass,
      const std::vector<FunctionStmtPtr>& methods) noexcept
    : name_(name), superclass_(superclass), methods_(methods) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class FunctionStmt final
  : public Stmt, public std::enable_shared_from_this<FunctionStmt> {
  Token name_;
  std::vector<Token> params_;
  std::vector<StmtPtr> body_;
public:
  inline const Token& name(void) const { return name_; }
  inline const std::vector<Token>& params(void) const { return params_; }
  inline const std::vector<StmtPtr>& body(void) const { return body_; }

  FunctionStmt(const Token& name,
      const std::vector<Token>& params,
      const std::vector<StmtPtr>& body) noexcept
    : name_(name), params_(params), body_(body) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class VarStmt final
  : public Stmt, public std::enable_shared_from_this<VarStmt> {
  Token name_;
  ExprPtr expr_;
public:
  inline const Token& name(void) const { return name_; }
  inline const ExprPtr& expr(void) const { return expr_; }

  VarStmt(const Token& name, const ExprPtr& expr) noexcept
    : name_(name), expr_(expr) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class BlockStmt final
  : public Stmt, public std::enable_shared_from_this<BlockStmt> {
  std::vector<StmtPtr> stmts_;
public:
  inline const std::vector<StmtPtr>& stmts(void) const { return stmts_; }

  BlockStmt(const std::vector<StmtPtr>& stmts) noexcept
    : stmts_(stmts) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class ExprStmt final
  : public Stmt, public std::enable_shared_from_this<ExprStmt> {
  ExprPtr expr_;
public:
  inline const ExprPtr& expr(void) const { return expr_; }

  ExprStmt(const ExprPtr& expr) noexcept
    : expr_(expr) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class WhileStmt final
  : public Stmt, public std::enable_shared_from_this<WhileStmt> {
  ExprPtr cond_;
  StmtPtr body_;
public:
  inline const ExprPtr& cond(void) const { return cond_; }
  inline const StmtPtr& body(void) const { return body_; }

  WhileStmt(const ExprPtr& cond, const StmtPtr& body) noexcept
    : cond_(cond), body_(body) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class IfStmt final
  : public Stmt, public std::enable_shared_from_this<IfStmt> {
  ExprPtr cond_;
  StmtPtr then_branch_;
  StmtPtr else_branch_;
public:
  inline const ExprPtr& cond(void) const { return cond_; }
  inline const StmtPtr& then_branch(void) const { return then_branch_; }
  inline const StmtPtr& else_branch(void) const { return else_branch_; }

  IfStmt(const ExprPtr& cond,
      const StmtPtr& then_branch, const StmtPtr& else_branch) noexcept
    : cond_(cond), then_branch_(then_branch), else_branch_(else_branch) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class PrintStmt final
  : public Stmt, public std::enable_shared_from_this<PrintStmt> {
  std::vector<ExprPtr> exprs_;
public:
  inline const std::vector<ExprPtr>& exprs(void) const { return exprs_; }

  PrintStmt(const std::vector<ExprPtr>& exprs) noexcept
    : exprs_(exprs) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

class ReturnStmt final
  : public Stmt, public std::enable_shared_from_this<ReturnStmt> {
  Token keyword_;
  ExprPtr value_;
public:
  inline const Token& keyword(void) const { return keyword_; }
  inline const ExprPtr& value(void) const { return value_; }

  ReturnStmt(const Token& keyword, const ExprPtr& value) noexcept
    : keyword_(keyword), value_(value) {
  }

  virtual void accept(const StmtVisitorPtr& visitor) override;
};

}
