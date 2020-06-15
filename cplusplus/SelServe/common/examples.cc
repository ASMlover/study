#include <iomanip>
#include <iostream>
#include <tuple>
#include <unordered_map>
#include "examples.hh"

namespace sser::examples {

// _ExampleContext        = Tuple[(Name: strv_t, Doc: strv_t, Fn: ExampleFn)]
using _ExampleContext     = std::tuple<strv_t, strv_t, ExampleFn>;
// _ExampleContextMap     = Map[strv_t, _ExampleContext]
using _ExampleContextMap  = std::unordered_map<strv_t, _ExampleContext>;
_ExampleContextMap* g_examples{};

static void example_usage() {
  std::cout << "USAGE: sserve [Alias]" << std::endl << std::endl;
  for (auto& e : *g_examples) {
    std::cout
      << std::left << std::setw(0) << e.first
      << " - "
      << std::get<1>(e.second)
      << std::endl;
  }
}

static void example_launch(strv_t key, const _ExampleContext& c) {
  std::cout
    << "========= [EXAMPLE: "
    << std::get<0>(c)
    << "(" << key << ")"
    << "] ========="
    << std::endl;
  std::get<2>(c)();
}

bool register_example(strv_t name, strv_t alias, strv_t doc, ExampleFn&& fn) {
  if (!g_examples)
    g_examples = new _ExampleContextMap;

  g_examples->insert({alias, {name, doc, std::move(fn)}});
  return true;
}

int launch_examples(strv_t alias) {
  if (!g_examples || g_examples->empty())
    return 0;

  if (!alias.empty()) {
    if (auto it = g_examples->find(alias); it != g_examples->end())
      example_launch(it->first, it->second);
  }
  else {
    for (auto& e : *g_examples)
      example_launch(e.first, e.second);
  }

  return 0;
}

}

SSER_EXAMPLE(Usage, help, "display usage for all examples") {
  sser::examples::example_usage();
}