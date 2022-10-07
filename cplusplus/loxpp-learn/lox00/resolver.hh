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
#include "common.hh"
#include "expr.hh"
#include "stmt.hh"

namespace loxpp::loxpp::interpret {

class Interpreter;
using InterpreterPtr = std::shared_ptr<Interpreter>;

}

namespace loxpp::resolver {

class Resolver final
  : public expr::Expr::Visitor
  , public stmt::Stmt::Visitor
  , public std::enable_shared_from_this<Resolver> {
  interpret::InterpreterPtr interpreter_;

  virtual void visit_assign(const AssignPtr& expr) override {}
  virtual void visit_binary(const BinaryPtr& expr) override {}
  virtual void visit_call(const CallPtr& expr) override {}
  virtual void visit_get(const GetPtr& expr) override {}
  virtual void visit_grouping(const GroupingPtr& expr) override {}
  virtual void visit_literal(const LiteralPtr& expr) override {}
  virtual void visit_logical(const LogicalPtr& expr) override {}
  virtual void visit_set(const SetPtr& expr) override {}
  virtual void visit_super(const SuperPtr& expr) override {}
  virtual void visit_this(const ThisPtr& expr) override {}
  virtual void visit_unary(const UnaryPtr& expr) override {}
  virtual void visit_variable(const VariablePtr& expr) override {}

  virtual void visit_block(const BlockPtr& stmt) override {
  }

  virtual void visit_class(const ClassPtr& stmt) override {}
  virtual void visit_expression(const ExpressionPtr& stmt) override {}
  virtual void visit_function(const FunctionPtr& stmt) override {}
  virtual void visit_if(const IfPtr& stmt) override {}
  virtual void visit_print(const PrintPtr& stmt) override {}
  virtual void visit_return(const ReturnPtr& stmt) override {}
  virtual void visit_var(const VarPtr& stmt) override {}
  virtual void visit_while(const WhilePtr& stmt) override {}
public:
  Resolver(const interpret::InterpreterPtr& interpreter) noexcept
    : interpreter_{interpreter} {
  }
};

}
