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
#pragma once

#include <memory>
#include "expr.h"
#include "value.h"

class Interpreter
  : public Visitor, public std::enable_shared_from_this<Interpreter> {
  Value value_;

  Value evaluate(const ExprPtr& expr);
public:
  virtual void visit_assign_expr(const AssignPtr& expr) override;
  virtual void visit_binary_expr(const BinaryPtr& expr) override;
  virtual void visit_call_expr(const CallPtr& expr) override;
  virtual void visit_get_expr(const GetPtr& expr) override;
  virtual void visit_set_expr(const SetPtr& expr) override;
  virtual void visit_grouping_expr(const GroupingPtr& expr) override;
  virtual void visit_literal_expr(const LiteralPtr& expr) override;
  virtual void visit_logical_expr(const LogicalPtr& expr) override;
  virtual void visit_super_expr(const SuperPtr& expr) override;
  virtual void visit_this_expr(const ThisPtr& expr) override;
  virtual void visit_unary_expr(const UnaryPtr& expr) override;
  virtual void visit_variable_expr(const VariablePtr& expr) override;
};
