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
#include "Types.hh"
#include "Expr.hh"

namespace ms {

class AstPrinter final
  : public ast::Expr::Visitor, public std::enable_shared_from_this<AstPrinter> {
  str_t printer_bytes_;

  void parenthesize(const str_t& name, std::initializer_list<ast::ExprPtr> exprs) noexcept;

  virtual void visit(const ast::AssignPtr& expr) override;
  virtual void visit(const ast::BinaryPtr& expr) override;
  virtual void visit(const ast::CallPtr& expr) override;
  virtual void visit(const ast::GetPtr& expr) override;
  virtual void visit(const ast::GroupingPtr& expr) override;
  virtual void visit(const ast::LiteralPtr& expr) override;
  virtual void visit(const ast::LogicalPtr& expr) override;
  virtual void visit(const ast::SetPtr& expr) override;
  virtual void visit(const ast::SuperPtr& expr) override;
  virtual void visit(const ast::ThisPtr& expr) override;
  virtual void visit(const ast::UnaryPtr& expr) override;
  virtual void visit(const ast::VariablePtr& expr) override;
public:
  str_t stringify(const ast::ExprPtr& expr) noexcept;
};

}
