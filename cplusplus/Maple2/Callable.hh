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
#include <unordered_map>
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
  virtual sz_t arity() const = 0;
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

  virtual sz_t arity() const override { return declaration_->params().size(); }
};

class Class final : public Callable, public std::enable_shared_from_this<Callable> {
  str_t name_;
  ClassPtr superclass_;
  std::unordered_map<str_t, FunctionPtr> methods_;
public:
  Class(const str_t& name, const ClassPtr& superclass,
      const std::unordered_map<str_t, FunctionPtr>& methods) noexcept
    : name_{name}
    , superclass_{superclass}
    , methods_{methods} {
  }

  inline const str_t& name() const noexcept { return name_; }

  inline FunctionPtr find_method(const str_t& name) const noexcept {
    if (auto it = methods_.find(name); it != methods_.end())
      return it->second;

    if (superclass_)
      return superclass_->find_method(name);
    return nullptr;
  }

  virtual Value call(const InterpreterPtr& interp, const std::vector<Value>& arguments) override;
  virtual str_t as_string() const override { return name_; }

  virtual sz_t arity() const override;
};

class Instance final : private UnCopyable, public std::enable_shared_from_this<Instance> {
  ClassPtr klass_;
  std::unordered_map<str_t, Value> fields_;
public:
  Instance(const ClassPtr& klass) noexcept : klass_{klass} {}

  str_t as_string() const noexcept;

  inline Value get(const Token& name)  {
    auto field_name = name.literal();
    if (auto it = fields_.find(field_name); it != fields_.end())
      return it->second;

    auto method = klass_->find_method(field_name);
    if (method)
      return Value{method->bind(shared_from_this())};

    throw RuntimeError(name, "Undefined property `" + field_name + "`.");
  }

  inline void set(const Token& name, const Value& value) noexcept {
    fields_[name.literal()] = value;
  }
};

}
