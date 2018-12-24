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
#include "expr.h"

Assign::Assign(const Token& name, const ExprPtr& value)
  : name_(name)
  , value_(value) {
}

void Assign::accept(const VisitorPtr& visitor) {
  visitor->visit_assign_expr(shared_from_this());
}

Binary::Binary(const ExprPtr& left, const Token& oper, const ExprPtr& right)
  : left_(left)
  , operator_(oper)
  , right_(right) {
}

void Binary::accept(const VisitorPtr& visitor) {
  visitor->visit_binary_expr(shared_from_this());
}

Call::Call(const ExprPtr& callee,
    const Token& paren, const std::vector<ExprPtr>& arguments)
  : callee_(callee)
  , paren_(paren)
  , arguments_(arguments) {
}

void Call::accept(const VisitorPtr& visitor) {
  visitor->visit_call_expr(shared_from_this());
}

Get::Get(const ExprPtr& object, const Token& name)
  : object_(object)
  , name_(name) {
}

void Get::accept(const VisitorPtr& visitor) {
  visitor->visit_get_expr(shared_from_this());
}

Set::Set(const ExprPtr& object, const Token& name, const ExprPtr& value)
  : object_(object)
  , name_(name)
  , value_(value) {
}

void Set::accept(const VisitorPtr& visitor) {
  visitor->visit_set_expr(shared_from_this());
}

Grouping::Grouping(const ExprPtr& expression)
  : expression_(expression) {
}

void Grouping::accept(const VisitorPtr& visitor) {
  visitor->visit_grouping_expr(shared_from_this());
}

Literal::Literal(const Value& value)
  : value_(value) {
}

void Literal::accept(const VisitorPtr& visitor) {
  visitor->visit_literal_expr(shared_from_this());
}

Logical::Logical(const ExprPtr& left, const Token& oper, const ExprPtr& right)
  : left_(left)
  , operator_(oper)
  , right_(right) {
}

void Logical::accept(const VisitorPtr& visitor) {
  visitor->visit_logical_expr(shared_from_this());
}

Super::Super(const Token& keyword, const Token& method)
  : keyword_(keyword)
  , method_(method) {
}

void Super::accept(const VisitorPtr& visitor) {
  visitor->visit_super_expr(shared_from_this());
}

This::This(const Token& keyword)
  : keyword_(keyword) {
}

void This::accept(const VisitorPtr& visitor) {
  visitor->visit_this_expr(shared_from_this());
}

Unary::Unary(const Token& oper, const ExprPtr& right)
  : operator_(oper)
  , right_(right) {
}

void Unary::accept(const VisitorPtr& visitor) {
  visitor->visit_unary_expr(shared_from_this());
}

Variable::Variable(const Token& name)
  : name_(name) {
}

void Variable::accept(const VisitorPtr& visitor) {
  visitor->visit_variable_expr(shared_from_this());
}
