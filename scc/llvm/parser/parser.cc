// Copyright (c) 2015 ASMlover. All rights reserved.
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
#include <memory>
#include <string>
#include <vector>

class Expr {
public:
  virtual ~Expr(void) {}
};
typedef std::unique_ptr<Expr> ExprPtr;
typedef std::vector<ExprPtr>  ExprVector;
typedef std::vector<std::string> > StringVector;

class NumberExpr : public Expr {
  double value_;
public:
  explicit NumberExpr(double value)
    : value_(value) {
  }
};

class VariableExpr : public Expr {
  std::string variable_;
public:
  explicit VariableExpr(const std::string& variable)
    : variable_(variable) {
  }
};

class BinaryExpr : public Expr {
  char op_;
  ExprPtr lhs_;
  ExprPtr rhs_;
public:
  BinaryExpr(char op, ExprPtr lhr, ExprPtr rhs)
    : op_(op)
    , lhs_(std::move(lhs))
    , rhs_(std::move(rhs)) {
  }
};

class CallExpr : public Expr {
  std::string callee_;
  ExprVector  args_;
public:
  CallExpr(const std::string& callee, const ExprVector& args)
    : callee_(callee)
    , args_(args) {
  }
};

class Prototype {
  std::string  name_;
  StringVector args_;
public:
  Prototype(const std::string& name, const StringVector& args)
    : name_(name)
    , args_(args) {
  }
};
typedef std::unique_ptr<Prototype> PrototypePtr;

class Function {
  PrototypePtr proto_;
  ExprPtr      body_;
public:
  Function(PrototypePtr proto, ExprPtr body)
    : proto_(std::move(proto))
    , body_(std::move(body)) {
  }
};
