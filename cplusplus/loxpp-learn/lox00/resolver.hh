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
#include <unordered_map>
#include "common.hh"
#include "expr.hh"
#include "stmt.hh"

namespace loxpp::interpret {

class Interpreter;
using InterpreterPtr = std::shared_ptr<Interpreter>;

}

namespace loxpp::resolver {

class Resolver final
  : public expr::Expr::Visitor
  , public stmt::Stmt::Visitor
  , public std::enable_shared_from_this<Resolver> {
  using ScopeMap = std::unordered_map<str_t, bool>;

  interpret::InterpreterPtr interpreter_;
  std::vector<ScopeMap> scopes_;

  inline void resolve(const expr::ExprPtr& expr) noexcept { expr->accept(shared_from_this()); }
  inline void resolve(const stmt::StmtPtr& stmt) noexcept { stmt->accept(shared_from_this()); }

  inline void resolve(const std::vector<stmt::StmtPtr>& statements) noexcept {
    for (const auto& stmt : statements)
      resolve(stmt);
  }

  inline void begin_scope() noexcept { scopes_.push_back({}); }
  inline void end_scope() noexcept { scopes_.pop_back(); }
private:
  virtual void visit_assign(const expr::AssignPtr& expr) override {}
  virtual void visit_binary(const expr::BinaryPtr& expr) override {}
  virtual void visit_call(const expr::CallPtr& expr) override {}
  virtual void visit_get(const expr::GetPtr& expr) override {}
  virtual void visit_grouping(const expr::GroupingPtr& expr) override {}
  virtual void visit_literal(const expr::LiteralPtr& expr) override {}
  virtual void visit_logical(const expr::LogicalPtr& expr) override {}
  virtual void visit_set(const expr::SetPtr& expr) override {}
  virtual void visit_super(const expr::SuperPtr& expr) override {}
  virtual void visit_this(const expr::ThisPtr& expr) override {}
  virtual void visit_unary(const expr::UnaryPtr& expr) override {}
  virtual void visit_variable(const expr::VariablePtr& expr) override {}

  virtual void visit_block(const stmt::BlockPtr& stmt) override {
  }

  virtual void visit_class(const stmt::ClassPtr& stmt) override {}
  virtual void visit_expression(const stmt::ExpressionPtr& stmt) override {}
  virtual void visit_function(const stmt::FunctionPtr& stmt) override {}
  virtual void visit_if(const stmt::IfPtr& stmt) override {}
  virtual void visit_print(const stmt::PrintPtr& stmt) override {}
  virtual void visit_return(const stmt::ReturnPtr& stmt) override {}
  virtual void visit_var(const stmt::VarPtr& stmt) override {}
  virtual void visit_while(const stmt::WhilePtr& stmt) override {}
public:
  Resolver(const interpret::InterpreterPtr& interpreter) noexcept
    : interpreter_{interpreter} {
  }
};

}
