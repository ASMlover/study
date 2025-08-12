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
#include <functional>
#include <vector>
#include "Common.hh"
#include "Value.hh"
#include "Environment.hh"

namespace ms {

class Function final : private UnCopyable {
  using FunctionType = std::function<Value (const std::vector<Value>&)>;

  std::vector<str_t> params_;
  EnvironmentPtr closure_;
  FunctionType body_;
  str_t name_;
public:
  Function(const std::vector<str_t>& params, EnvironmentPtr closure, FunctionType body, const str_t& name = "") noexcept
    : params_{params}, closure_{std::move(closure)}, body_{std::move(body)}, name_{name} {
  }

  Value call(const std::vector<Value>& args) noexcept(false) {
    if (args.size() != params_.size())
      throw std::runtime_error(std::format("Expected {} arguments but got {}", params_.size(), args.size()));

    return body_(args);
  }

  inline sz_t params_size() const noexcept { return params_.size(); }
};

}
