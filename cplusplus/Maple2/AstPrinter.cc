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
#include "AstPrinter.hh"

namespace ms {

str_t AstPrinter::stringify(const ast::ExprPtr& expr) noexcept {
  expr->accept(shared_from_this());
  return printer_bytes_;
}

void AstPrinter::parenthesize(const str_t& name, std::initializer_list<ast::ExprPtr> exprs) noexcept {
  printer_bytes_ += "(";
  printer_bytes_ += name;

  for (auto& expr : exprs) {
    printer_bytes_ += " ";
    expr->accept(shared_from_this());
  }
  printer_bytes_ += ")";
}

void AstPrinter::visit(const ast::AssignPtr& expr) {
  auto name = str_t{"= "};
  name += expr->name().as_string();
  parenthesize(name, {expr->value()});
}

void AstPrinter::visit(const ast::BinaryPtr& expr) {
  parenthesize(expr->op().literal(), {expr->left(), expr->right()});
}

void AstPrinter::visit(const ast::CallPtr& expr) {}
void AstPrinter::visit(const ast::GetPtr& expr) {}

void AstPrinter::visit(const ast::GroupingPtr& expr) {
  parenthesize("group", {expr->expression()});
}

void AstPrinter::visit(const ast::LiteralPtr& expr) {}
void AstPrinter::visit(const ast::LogicalPtr& expr) {}
void AstPrinter::visit(const ast::SetPtr& expr) {}
void AstPrinter::visit(const ast::SuperPtr& expr) {}
void AstPrinter::visit(const ast::ThisPtr& expr) {}
void AstPrinter::visit(const ast::UnaryPtr& expr) {}
void AstPrinter::visit(const ast::VariablePtr& expr) {}

}
