// Copyright (c) 2023 ASMlover. All rights reserved.
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
#include <tuple>
#include <vector>
#include <unordered_map>
#include "harness.hh"

namespace clox::harness {

class GlobalHarness final : public Singleton<GlobalHarness> {
  using Context     = std::tuple<strv_t, ClosureFn>;
  using ContextList = std::vector<Context>;

  ContextList harness_;
  std::unordered_map<strv_t, sz_t> harness_indexes_;
public:
  inline sz_t size() const noexcept { return harness_.size(); }

  inline bool append_harness(strv_t name, ClosureFn&& fn) noexcept {
    harness_.push_back({name, std::move(fn)});
    harness_indexes_.insert({name, harness_.size() - 1});
    return true;
  }
};

bool register_harness(strv_t name, ClosureFn&& fn) noexcept {
  return GlobalHarness::get_instance().append_harness(name, std::move(fn));
}

int run_all_harness() {
  return 0;
}

int run_harness_with_name(strv_t name) {
  return 0;
}

}
