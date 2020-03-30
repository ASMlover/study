#include <tuple>
#include "examples.hh"

namespace coext {

using _Context = std::tuple<strv_t, ExampleClosure>;
using _ContextMap = std::unordered_map<strv_t, _Context>;
_ContextMap* g_examples{};

bool register_example(strv_t name, strv_t alias, ExampleClosure&& closure) {
  if (!g_examples)
    g_examples = new _ContextMap;

  g_examples->insert(std::make_pair(alias, std::make_tuple(name, std::move(closure))));
  return true;
}

int launch_examples(strv_t name) {
  return 0;
}

}