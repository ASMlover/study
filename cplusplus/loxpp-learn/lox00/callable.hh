// Copyright (c) 2022 ASMlover. All rights reserved.
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

#include <vector>
#include "common.hh"
#include "value.hh"
#include "environment.hh"
#include "stmt.hh"

namespace loxpp::interpret { class Interpreter; }

namespace loxpp::callable {

using InterpreterPtr =  std::shared_ptr<interpret::Interpreter>;

interface Callable : private UnCopyable {
  virtual ~Callable() {}

  virtual value::Value call(const InterpreterPtr& interp, const std::vector<value::Value>& arguments) = 0;
  virtual sz_t arity() const = 0;
  virtual str_t as_string() const = 0;
};

class Function;
class Class;
using FunctionPtr = std::shared_ptr<Function>;
using ClassPtr    = std::shared_ptr<Class>;

class Function final : public Callable {
  stmt::FunctionPtr declaration_;
  env::EnvironmentPtr closure_;
public:
  Function(const stmt::FunctionPtr& declaration, const env::EnvironmentPtr& closre) noexcept
    : declaration_{declaration}
    , closure_{closre} {
  }

  virtual value::Value call(const InterpreterPtr& interp, const std::vector<value::Value>& arguments) override;
  virtual str_t as_string() const override;

  virtual sz_t arity() const override { return declaration_->params().size(); }
};

class Class final : public Callable, public std::enable_shared_from_this<Class> {
  str_t name_;
public:
  Class(const str_t& name) noexcept : name_{name} {}

  inline str_t name() const noexcept {  return name_; }

  virtual value::Value call(const InterpreterPtr& interp, const std::vector<value::Value>& arguments) override;
  virtual sz_t arity() const override { return 0; }
  virtual str_t as_string() const override { return name_; }
};

class Instance : private UnCopyable {
  ClassPtr klass_;
  std::unordered_map<str_t, value::Value> fields_;
public:
  Instance(const ClassPtr& klass) noexcept : klass_{klass} {}

  str_t as_string() const noexcept;

  inline const value::Value& get(const Token& name) const {
    if (auto it = fields_.find(name.literal()); it != fields_.end())
      return it->second;

    throw RuntimeError(name, "undefined property `" + name.literal() + "`");
  }

  inline void set(const Token& name, const value::Value& value) noexcept {
    fields_[name.literal()] = value;
  }
};

}
