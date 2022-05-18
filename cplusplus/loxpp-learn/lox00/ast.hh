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

#include <memory>
#include <vector>
#include "common.hh"
#include "token.hh"

namespace loxpp {

class Token;

namespace ast {

interface Expr;
interface Stmt;

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

class Assign;
class Binary;
class Call;
class Get;
class Grouping;
class Literal;
class Logical;
class Set;
class Super;
class This;
class Unary;
class Variable;

using AssignPtr = std::shared_ptr<Assign>;
using BinaryPtr = std::shared_ptr<Binary>;
using CallPtr = std::shared_ptr<Call>;
using GetPtr = std::shared_ptr<Get>;
using GroupingPtr = std::shared_ptr<Grouping>;
using LiteralPtr = std::shared_ptr<Literal>;
using LogicalPtr = std::shared_ptr<Logical>;
using SetPtr = std::shared_ptr<Set>;
using SuperPtr = std::shared_ptr<Super>;
using ThisPtr = std::shared_ptr<This>;
using UnaryPtr = std::shared_ptr<Unary>;
using VariablePtr = std::shared_ptr<Variable>;

interface Expr : private UnCopyable {
  interface Visitor : private UnCopyable {
    virtual void visit_assign(const AssignPtr& expr) = 0;
    virtual void visit_binary(const BinaryPtr& expr) = 0;
    virtual void visit_call(const CallPtr& expr) = 0;
    virtual void visit_get(const GetPtr& expr) = 0;
    virtual void visit_grouping(const GroupingPtr& expr) = 0;
    virtual void visit_literal(const LiteralPtr& expr) = 0;
    virtual void visit_logical(const LogicalPtr& expr) = 0;
    virtual void visit_set(const SetPtr& expr) = 0;
    virtual void visit_super(const SuperPtr& expr) = 0;
    virtual void visit_this(const ThisPtr& expr) = 0;
    virtual void visit_unary(const UnaryPtr& expr) = 0;
    virtual void visit_variable(const VariablePtr& expr) = 0;
  };
  using VisitorPtr = std::shared_ptr<Visitor>;

  virtual void accept(const VisitorPtr& visitor) = 0;
};

class Assign final : public Expr, public std::enable_shared_from_this<Assign> {
  Token name_;
  ExprPtr value_;
public:
  Assign(const Token& name, const ExprPtr& value) noexcept
    : name_{name}, value_{value} {
  }

  inline const Token& name() const noexcept { return name_; }
  inline const ExprPtr& value() const noexcept { return value_; }

  virtual void accept(const Expr::VisitorPtr& visitor) override;
};

class Binary final : public Expr, public std::enable_shared_from_this<Binary> {
  ExprPtr left_;
  Token oper_;
  ExprPtr right_;
public:
  Binary(const ExprPtr& left, const Token& oper, const ExprPtr& right) noexcept
    : left_{left}, oper_{oper}, right_{right} {
  }

  inline const ExprPtr& left() const noexcept { return left_; }
  inline const Token& oper() const noexcept { return oper_; }
  inline const ExprPtr& right() const noexcept { return right_; }

  virtual void accept(const Expr::VisitorPtr& visitor) override;
};

class Call final : public Expr, public std::enable_shared_from_this<Call> {
  ExprPtr callee_;
  Token paren_;
  std::vector<ExprPtr> arguments_;
public:
  Call(const ExprPtr& callee, const Token& paren, const std::vector<ExprPtr>& arguments) noexcept
    : callee_{callee}, paren_{paren}, arguments_{arguments} {
  }

  inline const ExprPtr& callee() const noexcept { return callee_; }
  inline const Token& paren() const noexcept { return paren_; }
  inline const std::vector<ExprPtr>& arguments() const noexcept { return arguments_; }

  virtual void accept(const Expr::VisitorPtr& visitor) override;
};

class Get final : public Expr, public std::enable_shared_from_this<Get> {
  ExprPtr object_;
  Token name_;
public:
  Get(const ExprPtr& object, const Token& name) noexcept
    : object_{object}, name_{name} {
  }

  inline const ExprPtr& object() const noexcept { return object_; }
  inline const Token& name() const noexcept { return name_; }

  virtual void accept(const Expr::VisitorPtr& visitor) override;
};

}}
