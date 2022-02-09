// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include <Common/Colorful.hh>
#include <Common/Harness.hh>

namespace Tadpole::Common::Harness {

using Context     = std::tuple<strv_t, ClosureFn>;
using ContextList = std::vector<Context>;

ContextList* g_harness{};
std::unordered_map<strv_t, sz_t> g_harness_indexes;

bool register_harness(strv_t name, ClosureFn&& fn) {
  if (!g_harness)
    g_harness = new ContextList;

  g_harness->push_back({name, std::move(fn)});
  g_harness_indexes.insert({name, g_harness->size() - 1});

  return true;
}

int run_all_harness() {
  sz_t total_tests{};
  sz_t passed_tests{};

  if (g_harness && !g_harness->empty()) {
    total_tests = g_harness->size();

    for (auto& hc : *g_harness) {
      auto [hc_name, hc_fn] = hc;
      hc_fn();
      ++passed_tests;

      std::cout
        << Colorful::fg::green
        << "********* [" << hc_name << "] test Harness PASSED "
        << "(" << passed_tests << "/" << total_tests << ") *********"
        << Colorful::reset << std::endl;
    }
  }

  std::cout
    << Colorful::fg::green
    << "========= PASSED " << "(" << passed_tests << "/" << total_tests << ") test Harness ========="
    << Colorful::reset << std::endl;

  return 0;
}

int run_harness_with(strv_t name) {
  if (!g_harness || g_harness->empty())
    return 0;

  if (auto it = g_harness_indexes.find(name); it != g_harness_indexes.end()) {
    auto& hc = g_harness->at(it->second);
    auto [_, hc_fn] = hc;
    hc_fn();

    std::cout
      << Colorful::fg::green
      << "********* Run [" << name << "] Test Harness PASSED *********"
      << Colorful::reset
      << std::endl;
  }
  return 0;
}

}
