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
#pragma once

#include <memory>
#include <vector>
#include "token.h"

template <typename T>
using Visitable = std::enable_shared_from_this<T>;

struct Visitor;
struct Expr;

using VisitorPtr = std::shared_ptr<Visitor>;
using ExprPtr = std::shared_ptr<Expr>;

struct Expr {
  virtual void accept(const VisitorPtr& visitor) = 0;
};

class Assign : public Expr, public Visitable<Assign> {
  Token name_;
  ExprPtr value_;
public:
  Assign(const Token& name, const ExprPtr& value);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Binary : public Expr, public Visitable<Binary> {
  ExprPtr left_;
  Token operator_;
  ExprPtr right_;
public:
  Binary(const ExprPtr& left, const Token& oper, const ExprPtr& right);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Call : public Expr, public Visitable<Call> {
  ExprPtr callee_;
  Token paren_;
  std::vector<ExprPtr> arguments_;
public:
  Call(const ExprPtr& callee,
      const Token& paren, const std::vector<ExprPtr>& arguments);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Get : public Expr, public Visitable<Get> {
  ExprPtr object_;
  Token name_;
public:
  Get(const ExprPtr& object, const Token& name);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Set : public Expr, public Visitable<Set> {
  ExprPtr object_;
  Token name_;
  ExprPtr value_;
public:
  Set(const ExprPtr& object, const Token& name, const ExprPtr& value);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Grouping : public Expr, public Visitable<Grouping> {
  ExprPtr expression_;
public:
  Grouping(const ExprPtr& expression);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Literal : public Expr, public Visitable<Literal> {
  Token value_;
public:
  Literal(const Token& value);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Logical : public Expr, public Visitable<Logical> {
  ExprPtr left_;
  Token operator_;
  ExprPtr right_;
public:
  Logical(const ExprPtr& left, const Token& oper, const ExprPtr& right);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Super : public Expr, public Visitable<Super> {
  Token keyword_;
  Token method_;
public:
  Super(const Token& keyword, const Token& method);
  virtual void accept(const VisitorPtr& visitor) override;
};

class This : public Expr, public Visitable<This> {
  Token keyword_;
public:
  This(const Token& keyword);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Unary : public Expr, public Visitable<Unary> {
  Token operator_;
  ExprPtr right_;
public:
  Unary(const Token& oper, const ExprPtr& right);
  virtual void accept(const VisitorPtr& visitor) override;
};

class Variable : public Expr, public Visitable<Variable> {
  Token name_;
public:
  Variable(const Token& name);
  virtual void accept(const VisitorPtr& visitor) override;
};

struct Visitor {
  using AssignPtr = std::shared_ptr<Assign>;
  using BinaryPtr = std::shared_ptr<Binary>;
  using CallPtr = std::shared_ptr<Call>;
  using GetPtr = std::shared_ptr<Get>;
  using SetPtr = std::shared_ptr<Set>;
  using GroupingPtr = std::shared_ptr<Grouping>;
  using LiteralPtr = std::shared_ptr<Literal>;
  using LogicalPtr = std::shared_ptr<Logical>;
  using SuperPtr = std::shared_ptr<Super>;
  using ThisPtr = std::shared_ptr<This>;
  using UnaryPtr = std::shared_ptr<Unary>;
  using VariablePtr = std::shared_ptr<Variable>;

  virtual void visit_assign_expr(const AssignPtr& expr) = 0;
  virtual void visit_binary_expr(const BinaryPtr& expr) = 0;
  virtual void visit_call_expr(const CallPtr& expr) = 0;
  virtual void visit_get_expr(const GetPtr& expr) = 0;
  virtual void visit_set_expr(const SetPtr& expr) = 0;
  virtual void visit_grouping_expr(const GroupingPtr& expr) = 0;
  virtual void visit_literal_expr(const LiteralPtr& expr) = 0;
  virtual void visit_logical_expr(const LogicalPtr& expr) = 0;
  virtual void visit_super_expr(const SuperPtr& expr) = 0;
  virtual void visit_this_expr(const ThisPtr& expr) = 0;
  virtual void visit_unary_expr(const UnaryPtr& expr) = 0;
  virtual void visit_variable_expr(const VariablePtr& expr) = 0;
};
