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
#include <vector>
#include "../common/common.hh"
#include "../common/value.hh"
#include "../syntax/ast.hh"

namespace sage {

class Token;
class Environment;
class Interpreter;
class Function;
class Class;
class Instance;

using EnvironmentPtr = std::shared_ptr<Environment>;
using InterpreterPtr = std::shared_ptr<Interpreter>;
using FunctionPtr = std::shared_ptr<Function>;
using ClassPtr = std::shared_ptr<Class>;
using InstancePtr = std::shared_ptr<Instance>;

struct Callable : private UnCopyable {
  virtual ~Callable(void) {}
  virtual bool check_arity(void) const { return true; }
  virtual std::string name(void) const { return "callable"; }

  virtual Value call(
      const InterpreterPtr& interp, const std::vector<Value>& arguments) = 0;
  virtual std::size_t arity(void) const = 0;
  virtual std::string to_string(void) const = 0;
};

class Function
  : public Callable, public std::enable_shared_from_this<Function> {
  FunctionStmtPtr decl_;
  EnvironmentPtr closure_;
public:
  Function(const FunctionStmtPtr& decl, const EnvironmentPtr& closure);

  virtual std::string name(void) const override;
  virtual Value call(const InterpreterPtr& interp,
      const std::vector<Value>& arguments) override;
  virtual std::size_t arity(void) const override;
  virtual std::string to_string(void) const override;
};

class Class
  : public Callable, public std::enable_shared_from_this<Class> {
  std::string name_;
public:
  Class(const std::string& name);

  virtual std::string name(void) const override;
  virtual Value call(const InterpreterPtr& interp,
      const std::vector<Value>& arguments) override;
  virtual std::size_t arity(void) const override;
  virtual std::string to_string(void) const override;
};

class Instance
  : private UnCopyable, public std::enable_shared_from_this<Instance> {
  ClassPtr class_;
public:
  Instance(const ClassPtr& cls);

  bool is_truthy(void) const;
  std::string to_string(void) const;
  void set_property(const Token& name, const Value& value);
  Value get_property(const Token& name);
};

}
