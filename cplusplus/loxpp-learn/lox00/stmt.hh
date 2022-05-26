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
#include "expr.hh"

namespace loxpp::stmt {

class Block;
class Class;
class Expression;
class Function;
class If;
class Print;
class Return;
class Var;
class While;

using BlockPtr = std::shared_ptr<Block>;
using ClassPtr = std::shared_ptr<Class>;
using ExpressionPtr = std::shared_ptr<Expression>;
using FunctionPtr = std::shared_ptr<Function>;
using IfPtr = std::shared_ptr<If>;
using PrintPtr = std::shared_ptr<Print>;
using ReturnPtr = std::shared_ptr<Return>;
using VarPtr = std::shared_ptr<Var>;
using WhilePtr = std::shared_ptr<While>;

struct Stmt : private UnCopyable {
  interface Visitor : private UnCopyable {
    virtual void visit_block(const BlockPtr& stmt) = 0;
    virtual void visit_class(const ClassPtr& stmt) = 0;
    virtual void visit_expression(const ExpressionPtr& stmt) = 0;
    virtual void visit_function(const FunctionPtr& stmt) = 0;
    virtual void visit_if(const IfPtr& stmt) = 0;
    virtual void visit_print(const PrintPtr& stmt) = 0;
    virtual void visit_return(const ReturnPtr& stmt) = 0;
    virtual void visit_var(const VarPtr& stmt) = 0;
    virtual void visit_while(const WhilePtr& stmt) = 0;
  };
  using VisitorPtr = std::shared_ptr<Visitor>;

  virtual void accept(const VisitorPtr& visitor) = 0;
};
using StmtPtr = std::shared_ptr<Stmt>;

class Block final : public Stmt, public std::enable_shared_from_this<Block> {
  std::vector<StmtPtr> statements_;
public:
  Block(const std::vector<StmtPtr>& statements) noexcept
    : statements_{statements} {
  }

  inline const std::vector<StmtPtr>& statements() const noexcept { return statements_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit_block(shared_from_this());
  }
};

class Class final : public Stmt, public std::enable_shared_from_this<Class> {
  Token name_;
  expr::VariablePtr superclass_;
  std::vector<FunctionPtr> methods_;
public:
  Class(const Token& name,
      const expr::VariablePtr& superclass, const std::vector<FunctionPtr>& methods) noexcept
    : name_{name}, superclass_{superclass}, methods_{methods} {
  }

  inline const Token& name() const noexcept { return name_; }
  inline const expr::VariablePtr& superclass() const noexcept { return superclass_; }
  inline const std::vector<FunctionPtr> methods() const noexcept { return methods_; }

  virtual void accept(const Stmt::VisitorPtr& visitor) override {
    visitor->visit_class(shared_from_this());
  }
};

}
