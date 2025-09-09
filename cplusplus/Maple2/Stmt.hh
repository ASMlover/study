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
#include "Common.hh"
#include "Token.hh"
#include "Expr.hh"

namespace ms::ast {

class Block;
class Class;
class Expression;
class Function;
class If;
class Import;
class Print;
class Return;
class Var;
class While;

using BlockPtr      = std::shared_ptr<Block>;
using ClassPtr      = std::shared_ptr<Class>;
using ExpressionPtr = std::shared_ptr<Expression>;
using FunctionPtr   = std::shared_ptr<Function>;
using IfPtr         = std::shared_ptr<If>;
using ImportPtr     = std::shared_ptr<Import>;
using PrintPtr      = std::shared_ptr<Print>;
using ReturnPtr     = std::shared_ptr<Return>;
using VarPtr        = std::shared_ptr<Var>;
using WhilePtr      = std::shared_ptr<While>;

struct Stmt : private UnCopyable {
  interface Visitor : private UnCopyable {
    virtual void visit(const BlockPtr& stmt) = 0;
    virtual void visit(const ClassPtr& stmt) = 0;
    virtual void visit(const ExpressionPtr& stmt) = 0;
    virtual void visit(const FunctionPtr& stmt) = 0;
    virtual void visit(const IfPtr& stmt) = 0;
    virtual void visit(const ImportPtr& stmt) = 0;
    virtual void visit(const PrintPtr& stmt) = 0;
    virtual void visit(const ReturnPtr& stmt) = 0;
    virtual void visit(const VarPtr& stmt) = 0;
    virtual void visit(const WhilePtr& stmt) = 0;
  };
  using VisitorPtr = std::shared_ptr<Visitor>;

  virtual void accept(const VisitorPtr& visitor) = 0;
};
using StmtPtr = std::shared_ptr<Stmt>;

class Block final : public Stmt, public std::enable_shared_from_this<Block> {
  std::vector<StmtPtr> statements_;
public:
  Block(const std::vector<StmtPtr>& statements) noexcept : statements_{statements} {}

  inline const std::vector<StmtPtr>& statements() const noexcept { return statements_; }

  template <typename Fn> inline void iter_statements(Fn&& fn) noexcept {
    for (const auto& stmt : statements_)
      fn(stmt);
  }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class Class final : public Stmt, public std::enable_shared_from_this<Class> {
  Token name_;
  VariablePtr superclass_;
  std::vector<FunctionPtr> methods_;
public:
  Class(const Token& name, const VariablePtr& superclass, const std::vector<FunctionPtr>& methods) noexcept
    : name_{name}, superclass_{superclass}, methods_{methods} {
  }

  inline const Token& name() const noexcept { return name_; }
  inline const VariablePtr& superclass() const noexcept { return superclass_; }
  inline const std::vector<FunctionPtr>& methods() const noexcept { return methods_; }

  template <typename Fn> inline void iter_methods(Fn&& fn) noexcept {
    for (const auto& meth : methods_)
      fn(meth);
  }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class Expression final : public Stmt, public std::enable_shared_from_this<Expression> {
  ExprPtr expression_;
public:
  Expression(const ExprPtr& expression) noexcept : expression_{expression} {}

  inline const ExprPtr& expression() const noexcept { return expression_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class Function final : public Stmt, public std::enable_shared_from_this<Function> {
  Token name_;
  std::vector<Token> params_;
  std::vector<StmtPtr> body_;
public:
  Function(const Token& name, const std::vector<Token>& params, const std::vector<StmtPtr>& body) noexcept
    : name_{name}, params_{params}, body_{body} {
  }

  inline const Token& name() const noexcept { return name_; }
  inline const std::vector<Token>& params() const noexcept { return params_; }
  inline const std::vector<StmtPtr>& body() const noexcept { return body_; }

  template <typename Fn> inline void iter_parameters(Fn&& fn) noexcept {
    for (const auto& param : params_)
      fn(param);
  }

  template <typename Fn> inline void iter_statements(Fn&& fn) noexcept {
    for (const auto& stmt : body_)
      fn(stmt);
  }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class If final : public Stmt, public std::enable_shared_from_this<If> {
  ExprPtr condition_;
  StmtPtr then_branch_;
  StmtPtr else_branch_;
public:
  If(const ExprPtr& cond, const StmtPtr& then_branch, const StmtPtr& else_branch) noexcept
    : condition_{cond}, then_branch_{then_branch}, else_branch_{else_branch} {
  }

  inline const ExprPtr& condition() const noexcept { return condition_; }
  inline const StmtPtr& then_branch() const noexcept { return then_branch_; }
  inline const StmtPtr& else_branch() const noexcept { return else_branch_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class Import final : public Stmt, public std::enable_shared_from_this<Import> {
  Token module_path_;
public:
  Import(const Token& module_path) noexcept : module_path_{module_path} {}

  inline const Token& module_path() const noexcept { return module_path_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class Print final : public Stmt, public std::enable_shared_from_this<Print> {
  ExprPtr expression_;
public:
  Print(const ExprPtr& expression) noexcept : expression_{expression} {}

  inline const ExprPtr& expression() const noexcept { return expression_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class Return final : public Stmt, public std::enable_shared_from_this<Return> {
  Token keyword_;
  ExprPtr value_;
public:
  Return(const Token& keyword, const ExprPtr& value) noexcept
    : keyword_{keyword}, value_{value} {
  }

  inline const Token& keyword() const noexcept { return keyword_; }
  inline const ExprPtr& value() const noexcept { return value_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class Var final : public Stmt, public std::enable_shared_from_this<Var> {
  Token name_;
  ExprPtr initializer_;
public:
  Var(const Token& name, const ExprPtr& initializer) noexcept
    : name_{name}, initializer_{initializer} {
  }

  inline const Token& name() const noexcept { return name_; }
  inline const ExprPtr& initializer() const noexcept { return initializer_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

class While final : public Stmt, public std::enable_shared_from_this<While> {
  ExprPtr condition_;
  StmtPtr body_;
public:
  While(const ExprPtr& cond, const StmtPtr& body) noexcept : condition_{cond}, body_{body} {}

  inline const ExprPtr& condition() const noexcept { return condition_; }
  inline const StmtPtr& body() const noexcept { return body_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit(shared_from_this());
  }
};

}
