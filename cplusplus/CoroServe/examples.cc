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
#include <iomanip>
#include <tuple>
#include "examples.hh"

namespace coro {

using _Context    = std::tuple<strv_t, strv_t, ExampleClosure>;
using _ContextMap = std::unordered_map<strv_t, _Context>;
_ContextMap* g_examples{};

static void usage_examples() {
  std::cout << "USAGE: CoroServe [alias]" << std::endl << std::endl;
  for (auto& e : *g_examples) {
    std::cout
      << std::left
      << std::setw(8)
      << e.first
      << " - "
      << std::get<1>(e.second)
      << std::endl;
  }
}

bool register_example(
    strv_t name, strv_t alias, strv_t doc, ExampleClosure&& closure) {
  if (!g_examples)
    g_examples = new _ContextMap;

  g_examples->insert(
      std::make_pair(alias, std::make_tuple(name, doc, std::move(closure))));
  return true;
}

int launch_examples(strv_t alias) {
  if (!g_examples || g_examples->empty())
    return 0;

  if (!alias.empty()) {
    if (auto it = g_examples->find(alias); it != g_examples->end()) {
      std::cout
        << "========= [EXAMPLE: "
        << std::get<0>(it->second)
        << "("
        << alias
        << ")] ========="
        << std::endl;
      std::get<2>(it->second)();
    }
  }
  else {
    for (auto& e : *g_examples) {
      std::cout
        << "========= [EXAMPLE: "
        << std::get<0>(e.second)
        << "("
        << e.first
        << ")] ========="
        << std::endl;
      std::get<2>(e.second)();
    }
  }

  return 0;
}

}

CORO_EXAMPLE(USAGE, help, "display usage for all examples") {
  coro::usage_examples();
}
