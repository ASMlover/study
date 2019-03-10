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
#include "../common/errors.hh"
#include "../lex/token.hh"
#include "environment.hh"

namespace sage {

Environment::Environment(void) {
}

Environment::Environment(const EnvironmentPtr& enclosing)
  : enclosing_(enclosing) {
}

EnvironmentPtr Environment::ancestor(int distance) {
  auto envp(shared_from_this());
  for (int i = 0; i < distance; ++i)
    envp = envp->enclosing_;
  return envp;
}

EnvironmentPtr Environment::get_enclosing(void) const {
  return enclosing_;
}

void Environment::define(const std::string& name, const Value& value) {
  values_[name] = value;
}

void Environment::define(const Token& name, const Value& value) {
  values_[name.get_literal()] = value;
}

const Value& Environment::get(const std::string& name) const {
  return get(RuntimeError::virtual_token(name));
}

const Value& Environment::get(const Token& name) const {
  auto value_iter = values_.find(name.get_literal());
  if (value_iter != values_.end())
    return value_iter->second;

  if (enclosing_)
    return enclosing_->get(name);

  throw RuntimeError(name, "undefined variable `" + name.get_literal() + "`");
}

const Value& Environment::get_at(int distance, const std::string& name) {
  return ancestor(distance)->get(name);
}

const Value& Environment::get_at(int distance, const Token& name) {
  return ancestor(distance)->get(name);
}

void Environment::assign(const std::string& name, const Value& value) {
  assign(RuntimeError::virtual_token(name), value);
}

void Environment::assign(const Token& name, const Value& value) {
  auto value_iter = values_.find(name.get_literal());
  if (value_iter != values_.end()) {
    value_iter->second = value;
    return;
  }

  if (enclosing_) {
    enclosing_->assign(name, value);
    return;
  }

  throw RuntimeError(name, "undefined variable `" + name.get_literal() + "`");
}

void Environment::assign_at(
    int distance, const std::string& name, const Value& value) {
  ancestor(distance)->assign(name, value);
}

void Environment::assign_at(
    int distance, const Token& name, const Value& value) {
  ancestor(distance)->assign(name, value);
}

}
