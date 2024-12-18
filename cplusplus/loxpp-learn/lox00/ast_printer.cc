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
#include "ast_printer.hh"

namespace loxpp::printer {

str_t AstPrinter::stringify(const expr::ExprPtr& expr) noexcept {
  expr->accept(shared_from_this());
  return printer_bytes_;
}

void AstPrinter::parenthesize(
    const str_t& name, const std::initializer_list<expr::ExprPtr>& exprs) noexcept {
  printer_bytes_ += "(";
  printer_bytes_ += name;
  for (auto& expr : exprs) {
    printer_bytes_ += " ";
    expr->accept(shared_from_this());
  }
  printer_bytes_ += ")";
}

void AstPrinter::visit_assign(const expr::AssignPtr& expr) {
  str_t name("= ");
  name += expr->name().literal();
  parenthesize(name, {expr->value()});
}

void AstPrinter::visit_binary(const expr::BinaryPtr& expr) {
  parenthesize(expr->oper().literal(), {expr->left(), expr->right()});
}

void AstPrinter::visit_call(const expr::CallPtr& expr) {}
void AstPrinter::visit_get(const expr::GetPtr& expr) {}

void AstPrinter::visit_grouping(const expr::GroupingPtr& expr) {
  parenthesize("group", {expr->expression()});
}

void AstPrinter::visit_literal(const expr::LiteralPtr& expr) {
  // TODO:
}

void AstPrinter::visit_logical(const expr::LogicalPtr& expr) {
  parenthesize(expr->oper().literal(), {expr->left(), expr->right()});
}

void AstPrinter::visit_set(const expr::SetPtr& expr) {}
void AstPrinter::visit_super(const expr::SuperPtr& expr) {}
void AstPrinter::visit_this(const expr::ThisPtr& expr) {}

void AstPrinter::visit_unary(const expr::UnaryPtr& expr) {
  parenthesize(expr->oper().literal(), {expr->right()});
}

void AstPrinter::visit_variable(const expr::VariablePtr& expr) {}

}
