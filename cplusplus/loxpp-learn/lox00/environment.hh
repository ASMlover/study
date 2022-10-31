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

#include <unordered_map>
#include "common.hh"
#include "token.hh"
#include "errors.hh"
#include "value.hh"

namespace loxpp::env {

class Environment;
using EnvironmentPtr = std::shared_ptr<Environment>;

class Environment final : private UnCopyable, public std::enable_shared_from_this<Environment> {
  EnvironmentPtr enclosing_{};
  std::unordered_map<str_t, value::Value> values_;

  inline const EnvironmentPtr& ancestor(int distance) noexcept {
    auto environment(shared_from_this());
    for (int i = 0; i < distance; ++i)
      environment = environment->enclosing_;
    return environment;
  }
public:
  Environment() noexcept {}
  Environment(EnvironmentPtr enclosing) noexcept : enclosing_{enclosing} {}

  inline void define(const str_t& name, const value::Value& value) noexcept {
    values_[name] = value;
  }

  inline void assign(const Token& name, const value::Value& value) noexcept(false) {
    if (auto it = values_.find(name.literal()); it != values_.end()) {
      values_[name.literal()] = value;
      return;
    }

    if (enclosing_) {
      enclosing_->assign(name, value);
      return;
    }

    throw RuntimeError(name, "undefined variable `" + name.literal() + "`");
  }

  inline const value::Value& get(const str_t& name) noexcept(false) {
    if (auto it = values_.find(name); it != values_.end())
      return it->second;

    if (enclosing_)
      return enclosing_->get(name);

    throw RuntimeError(Token::make_from_literal(name), "undefined varable `" + name + "`");
  }

  inline const value::Value& get(const Token& name) noexcept(false) {
    return get(name.literal());
  }

  inline const value::Value& get_at(int distance, const str_t& name) noexcept(false) {
    return ancestor(distance)->get(name);
  }

  inline const value::Value& get_at(int distance, const Token& name) noexcept(false) {
    return ancestor(distance)->get(name);
  }
};

}
