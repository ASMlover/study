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
}

bool register_example(strv_t name, strv_t alias, strv_t doc, ExampleFn&& fn) {
  if (!g_examples)
    g_examples = new _ContextMap;

  g_examples->insert({ alias, {name, doc, std::move(fn)} });
  return true;
}

int launch_examples(strv_t alias) {
  return 0;
}

}
