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
#pragma once

#include <memory>
#include "callable.h"
#include "stmt.h"

class Environment;
class LoxInstance;

class LoxFunction : public Callable {
  using FunctionStmtPtr = std::shared_ptr<FunctionStmt>;
  using EnvironmentPtr = std::shared_ptr<Environment>;
  using LoxFunctionPtr = std::shared_ptr<LoxFunction>;
  using LoxInstancePtr = std::shared_ptr<LoxInstance>;

  FunctionStmtPtr declaration_;
  EnvironmentPtr closure_;
  bool is_ctor_{};
public:
  LoxFunction(const FunctionStmtPtr& decl,
      const EnvironmentPtr& closure, bool is_ctor = false)
    : declaration_(decl)
    , closure_(closure)
    , is_ctor_(is_ctor) {
  }

  virtual Value call(
      const InterpreterPtr& interp, const std::vector<Value>& args) override;
  virtual int arity(void) const override;
  virtual std::string to_string(void) const override;

  LoxFunctionPtr bind(const LoxInstancePtr& inst);
};
