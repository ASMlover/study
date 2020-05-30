#include <tuple>
#include <vector>
#include "harness.hh"

namespace sel::harness {

using HarnessContext      = std::tuple<strv_t, HarnessClosure>;
using HarnessClosureList  = std::vector<HarnessContext>;

HarnessClosureList* g_harness{};

bool register_harness(strv_t name, HarnessClosure&& closure) noexcept {
  if (!g_harness)
    g_harness = new HarnessClosureList;

  g_harness->push_back({ name, std::move(closure) });
  return true;
}

int run_all_harness() {
  sz_t total_tests{};
  sz_t passed_tests{};

  if (g_harness && !g_harness->empty()) {
    total_tests = g_harness->size();

    for (auto& hc : *g_harness) {
      auto [hc_name, hc_closure] = hc;
      hc_closure();
      ++passed_tests;

      std::cout
        << "********* [" << hc_name << "] test harness PASSED "
        << "(" << passed_tests << "/" << total_tests << ") "
        << "*********"
        << std::endl;
    }
  }

  std::cout
    << "========= PASSED "
    << "(" << passed_tests << "/" << total_tests << ") "
    << "========="
    << std::endl;

  return 0;
}

}