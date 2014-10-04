// Copyright (c) 2014 ASMlover. All rights reserved.
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
#include "global.h"
#include "snippet.h"
#include "expr.h"

uint32_t Expr::var_index_ = 0;
Expr::VariableMap Expr::variables_;
Expr::BreakListStack Expr::end_;
Expr::ContinueListStack Expr::start_;
Expr::UintListStack Expr::scope_;

Expr::Expr(void)
  : left_(ExprPtr(nullptr))
  , right_(ExprPtr(nullptr)) {
}

Expr::~Expr(void) {
}

void Expr::PushResolve(void) {
  end_.push(std::list<BreakPtr>());
  start_.push(std::list<ContinuePtr>());
  scope_.push(std::list<uint32_t>());
}

void Expr::Resolve(FilePtr& out, uint32_t start_addr, uint32_t end_addr) {
  for (auto c : start_.top())
    c->Resolve(out, start_addr);
  start_.pop();

  for (auto b : end_.top())
    b->Resolve(out, end_addr);
  end_.pop();

  uint8_t cmd = static_cast<uint8_t>(OpCode::OPCODE_DEL);
  for (auto i : scope_.top()) {
  }
  scope_.pop();
}

OpCode Expr::GetType(const std::string& token) {
  OpCode code = OpCode::OPCODE_INVAL;

  if (token == "true" || token == "false")
    return OpCode::OPCODE_BOOL;
  else if (token.empty())
    return OpCode::OPCODE_STRING;

  for (auto c : token) {
    if (!isdigit(c) && '.' != c) {
      code = OpCode::OPCODE_STRING;
      break;
    }
    else if (code != OpCode::OPCODE_REAL) {
      code = OpCode::OPCODE_INT;
    }

    if ('.' == c)
      code = OpCode::OPCODE_REAL;
  }

  return code;
}

void Expr::Reset(void) {
  var_index_ = 0;
  variables_.clear();

  while (!end_.empty())
    end_.pop();

  while (!start_.empty())
    start_.pop();

  while (!scope_.empty())
    scope_.pop();
}

void Expr::EvalChildren(FilePtr& out) {
  if (left_)
    left_->Execute(out);

  if (right_)
    right_->Execute(out);
}
