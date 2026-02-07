// Copyright (c) 2026 ASMlover. All rights reserved.
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
#include "Common.hh"
#include "Token.hh"

namespace ms {

enum class ExprKind : i8_t {
  EXPR_LITERAL = 0,
  EXPR_GROUPING,
  EXPR_UNARY,
  EXPR_BINARY,
};

class Expr;
using ExprPtr = std::unique_ptr<Expr>;
using ExprList = std::vector<ExprPtr>;

class Expr : private UnCopyable {
  ExprKind kind_;
public:
  Expr(ExprKind kind) noexcept : kind_{kind} {}
  virtual ~Expr() {}

  inline ExprKind kind() const noexcept { return kind_; }
  virtual str_t debug_string() const noexcept = 0;
};

class LiteralExpr final : public Expr {
  Token value_;
public:
  LiteralExpr(const Token& value) noexcept
    : Expr{ExprKind::EXPR_LITERAL}, value_{value} {
  }

  inline const Token& value() const noexcept { return value_; }
  str_t debug_string() const noexcept override;
};

class GroupingExpr final : public Expr {
  ExprPtr expression_;
public:
  GroupingExpr(ExprPtr expression) noexcept
    : Expr{ExprKind::EXPR_GROUPING}, expression_{std::move(expression)} {
  }

  inline const Expr* expression() const noexcept { return expression_.get(); }
  str_t debug_string() const noexcept override;
};

class UnaryExpr final : public Expr {
  Token operator_;
  ExprPtr right_;
public:
  UnaryExpr(const Token& operator_token, ExprPtr right) noexcept
    : Expr{ExprKind::EXPR_UNARY}, operator_{operator_token}, right_{std::move(right)} {
  }

  inline const Token& operator_token() const noexcept { return operator_; }
  inline const Expr* right() const noexcept { return right_.get(); }
  str_t debug_string() const noexcept override;
};

class BinaryExpr final : public Expr {
  ExprPtr left_;
  Token operator_;
  ExprPtr right_;
public:
  BinaryExpr(ExprPtr left, const Token& operator_token, ExprPtr right) noexcept
    : Expr{ExprKind::EXPR_BINARY}, left_{std::move(left)}, operator_{operator_token}, right_{std::move(right)} {
  }

  inline const Expr* left() const noexcept { return left_.get(); }
  inline const Token& operator_token() const noexcept { return operator_; }
  inline const Expr* right() const noexcept { return right_.get(); }
  str_t debug_string() const noexcept override;
};

str_t debug_expr(const Expr* expr) noexcept;

}
