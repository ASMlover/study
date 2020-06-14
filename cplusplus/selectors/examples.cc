#include <iomanip>
#include <iostream>
#include <tuple>
#include <unordered_map>
#include "examples.hh"

namespace sel::examples {

using _Context    = std::tuple<strv_t, strv_t, ExampleFn>;
using _ContextMap = std::unordered_map<strv_t, _Context>;
_ContextMap* g_examples{};

static void usage_examples() {
  std::cout << "USAGE: selectors [alias]" << std::endl << std::endl;
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

static void launch_example(strv_t key, const _Context& c) {
  std::cout
    << "========= [EXAMPLE: "
    << std::get<0>(c)
    << "("
    << key
    << ")] ========="
    << std::endl;
  std::get<2>(c)();
}

bool register_example(strv_t name, strv_t alias, strv_t doc, ExampleFn&& fn) {
  if (!g_examples)
    g_examples = new _ContextMap;

  g_examples->insert({ alias, {name, doc, std::move(fn)} });
  return true;
}

int launch_examples(strv_t alias) {
  if (!g_examples || g_examples->empty())
    return 0;

  if (!alias.empty()) {
    if (auto it = g_examples->find(alias); it != g_examples->end())
      launch_example(it->first, it->second);
  }
  else {
    for (auto& e : *g_examples)
      launch_example(e.first, e.second);
  }

  return 0;
}

}

SEL_EXAMPLE(Usage, help, "display usage for all examples") {
  sel::examples::usage_examples();
}
