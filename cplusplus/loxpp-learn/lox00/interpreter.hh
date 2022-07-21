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

#include <iostream>
#include <memory>
#include "common.hh"
#include "errors.hh"
#include "expr.hh"
#include "stmt.hh"
#include "value.hh"

namespace loxpp::interpret {

class Interpreter final
  : public expr::Expr::Visitor
  , public stmt::Stmt::Visitor
  , public std::enable_shared_from_this<Interpreter> {
  ErrorReporter& err_reporter_;
  value::Value value_{};

  inline value::Value evaluate(const expr::ExprPtr& expr) noexcept {
    expr->accept(shared_from_this());
    return value_;
  }

  void check_numeric_operands(const Token& oper, const value::Value& left, const value::Value& right) {
    if (left.is_numeric() && right.is_numeric())
      return;
    throw RuntimeError(oper, "operands must be numerics.");
  }

  void check_plus_operands(const Token& oper, const value::Value& left, const value::Value& right) {
    if ((left.is_numeric() && right.is_numeric()) || (left.is_string() && right.is_string()))
      return;
    throw RuntimeError(oper, "operands must be two numerics or two strings.");
  }

  virtual void visit_assign(const expr::AssignPtr& expr) override {}

  virtual void visit_binary(const expr::BinaryPtr& expr) override {
    value::Value left = evaluate(expr->left());
    value::Value right = evaluate(expr->right());

    const Token& oper = expr->oper();
    switch (oper.type()) {
    case TokenType::TK_GT:
      check_numeric_operands(oper, left, right);
      value_ = left > right; break;
    case TokenType::TK_GE:
      check_numeric_operands(oper, left, right);
      value_ = left >= right; break;
    case TokenType::TK_LT:
      check_numeric_operands(oper, left, right);
      value_ = left < right; break;
    case TokenType::TK_LE:
      check_numeric_operands(oper, left, right);
      value_ = left <= right; break;
    case TokenType::TK_PLUS:
      check_plus_operands(oper, left, right);
      value_ = left + right; break;
    case TokenType::TK_MINUS:
      check_numeric_operands(oper, left, right);
      value_ = left - right; break;
    case TokenType::TK_SLASH:
      check_numeric_operands(oper, left, right);
      value_ = left / right; break;
    case TokenType::TK_STAR:
      check_numeric_operands(oper, left, right);
      value_ = left * right; break;
    case TokenType::TK_NE: value_ = left != right; break;
    case TokenType::TK_EQEQ: value_ = left == right; break;
    }
  }

  virtual void visit_call(const expr::CallPtr& expr) override {}
  virtual void visit_get(const expr::GetPtr& expr) override {}

  virtual void visit_grouping(const expr::GroupingPtr& expr) override {
    evaluate(expr->expression());
  }

  virtual void visit_literal(const expr::LiteralPtr& expr) override {
    value_ = expr->value();
  }

  virtual void visit_logical(const expr::LogicalPtr& expr) override {}
  virtual void visit_set(const expr::SetPtr& expr) override {}
  virtual void visit_super(const expr::SuperPtr& expr) override {}
  virtual void visit_this(const expr::ThisPtr& expr) override {}

  virtual void visit_unary(const expr::UnaryPtr& expr) override {
    value::Value right = evaluate(expr->right());

    switch (expr->oper().type()) {
    case TokenType::TK_MINUS:
      value_ = -right; break;
    case TokenType::TK_NE:
      value_ = !right; break;
    }
  }

  virtual void visit_variable(const expr::VariablePtr& expr) override {}

  virtual void visit_block(const stmt::BlockPtr& stmt) override {}
  virtual void visit_class(const stmt::ClassPtr& stmt) override {}

  virtual void visit_expression(const stmt::ExpressionPtr& stmt) override {
    evaluate(stmt->expression());
  }

  virtual void visit_function(const stmt::FunctionPtr& stmt) override {}
  virtual void visit_if(const stmt::IfPtr& stmt) override {}

  virtual void visit_print(const stmt::PrintPtr& stmt) override {
    value::Value value = evaluate(stmt->expression());
    std::cout << value << std::endl;
  }

  virtual void visit_return(const stmt::ReturnPtr& stmt) override {}
  virtual void visit_var(const stmt::VarPtr& stmt) override {}
  virtual void visit_while(const stmt::WhilePtr& stmt) override {}
public:
  Interpreter(ErrorReporter& err_reporter) noexcept : err_reporter_{err_reporter} {}

  void interpret(const expr::ExprPtr& expression) noexcept {
    try {
      value::Value value = evaluate(expression);
      std::cout << value << std::endl;
    }
    catch (const RuntimeError& err) {
      err_reporter_.error(err.token(), err.message());
    }
  }
};

}
