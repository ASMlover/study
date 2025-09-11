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
#include <vector>
#include "Common.hh"
#include "Value.hh"
#include "Environment.hh"
#include "Stmt.hh"

namespace ms {

class Interpreter;
class Function;
class Class;
class Instance;

using InterpreterPtr  = std::shared_ptr<Interpreter>;
using FunctionPtr     = std::shared_ptr<Function>;
using ClassPtr        = std::shared_ptr<Class>;
using InstancePtr     = std::shared_ptr<Instance>;

interface Callable : private UnCopyable {
  virtual ~Callable() {}

  virtual Value call(const InterpreterPtr& interp, const std::vector<Value>& arguments) = 0;
  virtual sz_t airty() const = 0;
  virtual str_t as_string() const = 0;
};

class Function final : public Callable {
  ast::FunctionPtr declaration_;
  EnvironmentPtr closure_;
  bool is_initializer_{};
public:
  Function(const ast::FunctionPtr& decl, const EnvironmentPtr& closure, bool is_initializer) noexcept
    : declaration_{decl}, closure_{closure}, is_initializer_{is_initializer} {
  }

  FunctionPtr bind(InstancePtr instance) noexcept;

  virtual Value call(const InterpreterPtr& interp, const std::vector<Value>& arguments) override;
  virtual str_t as_string() const override;

  virtual sz_t airty() const override { return declaration_->params().size(); }
};

}
