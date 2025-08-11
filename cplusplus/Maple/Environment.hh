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

#include <format>
#include <unordered_map>
#include "Common.hh"
#include "Value.hh"

namespace ms {

class Environment final : private UnCopyable {
  EnvironmentPtr parent_{};
  std::unordered_map<str_t, Value> variables_;
public:
  Environment() noexcept {}
  Environment(EnvironmentPtr parent) noexcept : parent_{std::move(parent)}  {}

  inline const Value& get(const str_t& name) const noexcept(false) {
    if (auto it = variables_.find(name); it != variables_.end())
      return it->second;

    if (parent_)
      return parent_->get(name);

    throw std::runtime_error(std::format("Undefined variable `{}`", name));
  }

  inline void set(const str_t& name, Value value) noexcept {
    variables_[name] = std::move(value);
  }

  inline void define(const str_t& name, Value value) noexcept {
    variables_.insert_or_assign(name, std::move(value));
  }
};

}
