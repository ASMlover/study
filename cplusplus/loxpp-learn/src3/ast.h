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
#include "common.h"

namespace lox {

// expression -> literal | unary | binary | grouping ;
// literal    -> INTEGER | DECIMAL | STRING | "true" | "false" | "nil" ;
// grouping   -> "(" expression ")" ;
// unary      -> ( "-" | "!" ) expression ;
// binary     -> expression operator expression ;
// operator   -> "==" | "!=" | "<" | "<=" | ">" | ">="
//            | "+" | "-" | "*" | "/" | "%"
//            | "+=" | "-=" | "*=" | "/=" | "%=" ;

class Value;
class Token;

struct Expr;
struct Stmt;
struct ExprVisitor;
struct StmtVisitor;

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;
using ExprVisitorPtr = std::shared_ptr<ExprVisitor>;
using StmtVisitorPtr = std::shared_ptr<StmtVisitor>;

struct Expr : private UnCopyable {
  virtual void accept(const ExprVisitorPtr& visitor) = 0;
};

class AssignExpr
  : public Expr, public std::enable_shared_from_this<AssignExpr> {
  const Token& name_;
  const ExprPtr& value_;
public:
  const Token& name(void) const { return name_; }
  const ExprPtr& value(void) const { return value_; }

  AssignExpr(const Token& name, const ExprPtr& value)
    : name_(name), value_(value) {
  }

  virtual void accept(const ExprVisitorPtr& visitor) override;
};
using AssignExprPtr = std::shared_ptr<AssignExpr>;

class BinaryExpr
  : public Expr, public std::enable_shared_from_this<BinaryExpr> {
  const ExprPtr& left_;
  const Token& oper_;
  const ExprPtr& right_;
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
  const ExprPtr& callee_;
  const Token& paren_;
  const std::vector<ExprPtr>& arguments_;
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
  const ExprPtr& object_;
  const Token& name_;
  const ExprPtr& value_;
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
  const ExprPtr& object_;
  const Token& name_;
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
  const ExprPtr& expression_;
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
  const Value& value_;
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
  const ExprPtr& left_;
  const Token& oper_;
  const ExprPtr& right_;
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
  const Token& keyword_;
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
  const Token& keyword_;
  const Token& method_;
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
  const Token& oper_;
  const ExprPtr& right_;
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
  const Token& name_;
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
  const std::vector<Token>& params_;
  const std::vector<StmtPtr>& body_;
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
  virtual void visit_assign_expr(const AssignExprPtr& expr) = 0;
  virtual void visit_binary_expr(const BinaryExprPtr& expr) = 0;
  virtual void visit_call_expr(const CallExprPtr& expr) = 0;
  virtual void visit_set_expr(const SetExprPtr& expr) = 0;
  virtual void visit_get_expr(const GetExprPtr& expr) = 0;
  virtual void visit_grouping_expr(const GroupingExprPtr& expr) = 0;
  virtual void visit_literal_expr(const LiteralExprPtr& expr) = 0;
  virtual void visit_logical_expr(const LogicalExprPtr& expr) = 0;
  virtual void visit_self_expr(const SelfExprPtr& expr) = 0;
  virtual void visit_super_expr(const SuperExprPtr& expr) = 0;
  virtual void visit_unary_expr(const UnaryExprPtr& expr) = 0;
  virtual void visit_variable_expr(const VariableExprPtr& expr) = 0;
  virtual void visit_function_expr(const FunctionExprPtr& expr) = 0;
};

}
