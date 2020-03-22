// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include "harness.hh"

namespace wrencc {

using HarnessContext = std::tuple<strv_t, strv_t, ClosureFn>;
std::vector<HarnessContext>* g_harness(nullptr);

bool register_harness(strv_t base, strv_t name, ClosureFn&& closure) {
  if (!g_harness)
    g_harness = new std::vector<HarnessContext>;

  g_harness->push_back(std::make_tuple(base, name, std::move(closure)));
  return true;
}

int run_all_harness() {
  int total_tests = 0;
  int passes_tests = 0;

  if (g_harness && !g_harness->empty()) {
    total_tests = Xt::as_type<int>(g_harness->size());

    for (auto& hc : *g_harness) {
      auto [_, hc_name, hc_fn] = hc;
      hc_fn();
      ++passes_tests;

      std::cout
        << "********* ["  << hc_name << "] test harness PASSED "
        << "(" << passes_tests << "/" << total_tests << ") "
        << "*********"
        << std::endl;
    }
  }

  std::cout
    << "========= PASSED "
    << "(" << passes_tests << "/" << total_tests << ") "
    << "test harness ========="
    << std::endl;

  return 0;
}

}
