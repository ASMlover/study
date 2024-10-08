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
#include <optional>
#include <vector>
#include <unordered_map>
#include "harness.hh"

namespace clox::harness {

using Context     = std::tuple<strv_t, ClosureFn>;
using ContextList = std::vector<Context>;

class GlobalHarness final : public Singleton<GlobalHarness> {
  ContextList harness_;
  std::unordered_map<strv_t, sz_t> harness_indexes_;
public:
  inline sz_t size() const noexcept { return harness_.size(); }
  inline bool is_empty() const noexcept { return harness_.empty(); }

  inline bool append_harness(strv_t name, ClosureFn&& fn) noexcept {
    harness_.push_back({name, std::move(fn)});
    harness_indexes_.insert({name, harness_.size() - 1});
    return true;
  }

  std::optional<Context> get_harness(strv_t name) const noexcept {
    if (auto it = harness_indexes_.find(name); it != harness_indexes_.end())
      return harness_.at(it->second);
    return {};
  }

  template <typename Fn> inline void iter_harness(Fn fn) noexcept {
    for (auto& hc : harness_) {
      auto [hc_name, hc_fn] = hc;
      fn(hc_name, hc_fn);
    }
  }
};

bool register_harness(strv_t name, ClosureFn&& fn) noexcept {
  return GlobalHarness::get_instance().append_harness(name, std::move(fn));
}

int run_all_harness() {
  if (GlobalHarness::get_instance().is_empty())
    return 0;

  sz_t total_tests = GlobalHarness::get_instance().size();
  sz_t passed_tests{};
  GlobalHarness::get_instance().iter_harness([&total_tests, &passed_tests](strv_t hc_name, ClosureFn hc_fn) {
        hc_fn();
        ++passed_tests;

        std::cout << "********* [" << hc_name << "] test Harness PASSED "
          << "(" << passed_tests << "/" << total_tests << ") *********"
          << std::endl;
      });
  std::cout << "========= PASSED " << "(" << passed_tests << "/" << total_tests << ") test Harness =========" << std::endl;

  return 0;
}

int run_harness_with_name(strv_t name) {
  if (GlobalHarness::get_instance().is_empty())
    return 0;

  if (auto hc = GlobalHarness::get_instance().get_harness(name); hc.has_value()) {
    auto [_, hc_fn] = *hc;
    hc_fn();

    std::cout << "********* Run [" << name << "] Test Harness PASSED *********" << std::endl;
  }
  return 0;
}

}
