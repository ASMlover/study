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
#include "token.h"
#include "ast_printer.h"

namespace lox {

void AstPrinter::parenthesize(
    const std::string& name, const std::initializer_list<ExprPtr>& exprs) {
  out_bytes_ += "(";
  out_bytes_ += name;
  for (auto& expr : exprs) {
    out_bytes_ += " ";
    expr->accept(shared_from_this());
  }
  out_bytes_ += ")";
}

std::string AstPrinter::stringify(const ExprPtr& expr) {
  expr->accept(shared_from_this());
  return out_bytes_;
}

void AstPrinter::visit_assign_expr(const AssignExprPtr& expr) {
}

void AstPrinter::visit_binary_expr(const BinaryExprPtr& expr) {
  parenthesize(expr->oper().get_literal(), {expr->left(), expr->right()});
}

void AstPrinter::visit_call_expr(const CallExprPtr& expr) {
}

void AstPrinter::visit_set_expr(const SetExprPtr& expr) {
}

void AstPrinter::visit_get_expr(const GetExprPtr& expr) {
}

void AstPrinter::visit_grouping_expr(const GroupingExprPtr& expr) {
  parenthesize("group", {expr->expression()});
}

void AstPrinter::visit_literal_expr(const LiteralExprPtr& expr) {
  out_bytes_ += expr->value().stringify();
}

void AstPrinter::visit_logical_expr(const LogicalExprPtr& expr) {
  parenthesize(expr->oper().get_literal(), {expr->left(), expr->right()});
}

void AstPrinter::visit_self_expr(const SelfExprPtr& expr) {
}

void AstPrinter::visit_super_expr(const SuperExprPtr& expr) {
}

void AstPrinter::visit_unary_expr(const UnaryExprPtr& expr) {
  parenthesize(expr->oper().get_literal(), {expr->right()});
}

void AstPrinter::visit_variable_expr(const VariableExprPtr& expr) {
}

void AstPrinter::visit_function_expr(const FunctionExprPtr& expr) {
}

}
