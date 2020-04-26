#include <tuple>
#include <vector>
#include <Tests/Harness.hh>

namespace _mevo {

using HarnessContext      = std::tuple<strv_t, strv_t, HarnessClosure>;
using HarnessContextList  = std::vector<HarnessContext>;

HarnessContextList* g_harness{};

bool register_harness(strv_t base, strv_t name, HarnessClosure&& closure) {
  if (!g_harness)
    g_harness = new HarnessContextList;

  g_harness->push_back(std::make_tuple(base, name, std::move(closure)));
  return true;
}

int run_all_harness() {
  sz_t total_tests{};
  sz_t passed_tests{};

  if (g_harness && !g_harness->empty()) {
    total_tests = g_harness->size();

    for (auto& hc : *g_harness) {
      auto [_, hc_name, hc_closure] = hc;
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
    << "test harness "
    << "========="
    << std::endl;

  return 0;
}

}