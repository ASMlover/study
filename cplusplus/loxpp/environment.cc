// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "runtime_error.h"
#include "environment.h"

EnvironmentPtr Environment::ancestor(int distance) {
  EnvironmentPtr envp(shared_from_this());
  for (int i = 0; i < distance; ++i)
    envp = envp->enclosing_;
  return envp;
}

void Environment::assign(const Token& name, const Value& value) {
  std::string key = name.get_lexeme();
  if (values_.find(key) != values_.end()) {
    values_[key] = value;
    return;
  }

  if (enclosing_) {
    enclosing_->assign(name, value);
    return;
  }

  throw new RuntimeError(name, "undefined variable `" + key + "` ...");
}

void Environment::assign_at(
    int distance, const Token& name, const Value& value) {
  ancestor(distance)->assign(name, value);
}

void Environment::define_var(const std::string& name, const Value& value) {
  values_[name] = value;
}

Value Environment::get(const Token& name) const {
  auto valit = values_.find(name.get_lexeme());
  if (valit != values_.end())
    return valit->second;

  if (enclosing_)
    return enclosing_->get(name);

  throw new RuntimeError(name,
      "undefined variable `" + name.get_lexeme() + "` ...");
}

Value Environment::get(const std::string& name) const {
  auto valit = values_.find(name);
  if (valit != values_.end())
    return valit->second;

  if (enclosing_)
    return enclosing_->get(name);

  return Value();
}

Value Environment::get_at(int distance, const Token& name) {
  return ancestor(distance)->get(name);
}

Value Environment::get_at(int distance, const std::string& name) {
  return ancestor(distance)->get(name);
}
