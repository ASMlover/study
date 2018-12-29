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
#include "interpreter.h"

Value Interpreter::evaluate(const ExprPtr& expr) {
  expr->accept(shared_from_this());
  return value_;
}

void Interpreter::visit_assign_expr(const AssignPtr& expr) {
}

void Interpreter::visit_binary_expr(const BinaryPtr& expr) {
  Value left = evaluate(expr->left_);
  Value right = evaluate(expr->right_);

  switch (expr->operator_.get_type()) {
  case TOKEN_MINUS: value_ = left - right; break;
  case TOKEN_STAR: value_ = left * right; break;
  case TOKEN_SLASH: value_ = left / right; break;
  }
}

void Interpreter::visit_call_expr(const CallPtr& expr) {
}

void Interpreter::visit_get_expr(const GetPtr& expr) {
}

void Interpreter::visit_set_expr(const SetPtr& expr) {
}

void Interpreter::visit_grouping_expr(const GroupingPtr& expr) {
  value_ = evaluate(expr->expression_);
}

void Interpreter::visit_literal_expr(const LiteralPtr& expr) {
  value_ = expr->value_;
}

void Interpreter::visit_logical_expr(const LogicalPtr& expr) {
}

void Interpreter::visit_super_expr(const SuperPtr& expr) {
}

void Interpreter::visit_this_expr(const ThisPtr& expr) {
}

void Interpreter::visit_unary_expr(const UnaryPtr& expr) {
  Value right = evaluate(expr->right_);

  switch (expr->operator_.get_type()) {
  case TOKEN_MINUS: value_ = -right; break;
  case TOKEN_BANG: value_ = !right; break;
  }
}

void Interpreter::visit_variable_expr(const VariablePtr& expr) {
}
