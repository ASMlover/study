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
#include <unordered_map>
#include "common.hh"
#include "token.hh"
#include "interpret_value.hh"
#include "interpret_ast.hh"

namespace loxcc::interpret {

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
  virtual sz_t arity(void) const { return 0; }

  virtual Value call(
      const InterpreterPtr& interp, const std::vector<Value>& args) = 0;
  virtual str_t stringify(void) const = 0;
};

class Function final
  : public Callable, public std::enable_shared_from_this<Function> {
  FunctionStmtPtr decl_;
  EnvironmentPtr closure_;
  bool is_ctor_{};
public:
  Function(const FunctionStmtPtr& decl,
      const EnvironmentPtr& closure, bool is_ctor = false) noexcept
    : decl_(decl), closure_(closure), is_ctor_(is_ctor) {
  }

  virtual Value call(
      const InterpreterPtr& interp, const std::vector<Value>& args) override;
  virtual sz_t arity(void) const override;
  virtual str_t stringify(void) const override;

  FunctionPtr bind(const InstancePtr& inst);
};

class Class final
  : public Callable, public std::enable_shared_from_this<Class> {
  str_t name_;
  ClassPtr superclass_;
  std::unordered_map<str_t, FunctionPtr> methods_;
public:
  Class(const str_t& name, const ClassPtr& superclass,
      const std::unordered_map<str_t, FunctionPtr>& methods) noexcept
    : name_(name), superclass_(superclass), methods_(methods) {
  }

  inline str_t name(void) const { return name_; }

  virtual Value call(
      const InterpreterPtr& interp, const std::vector<Value>& args) override;
  virtual sz_t arity(void) const override;
  virtual str_t stringify(void) const override;

  FunctionPtr get_method(const InstancePtr& inst, const str_t& name);
  FunctionPtr get_method(const InstancePtr& inst, const Token& name);
};

class Instance final
  : private UnCopyable, public std::enable_shared_from_this<Instance> {
  ClassPtr cls_;
  std::unordered_map<str_t, Value> attrs_;
public:
  Instance(const ClassPtr& cls) noexcept : cls_(cls) {}

  str_t stringify(void) const;
  void set_attr(const Token& name, const Value& value);
  Value get_attr(const Token& name);
};

}
