#include <iomanip>
#include <tuple>
#include "examples.hh"

namespace coext {

using _Context = std::tuple<strv_t, strv_t, ExampleClosure>;
using _ContextMap = std::unordered_map<strv_t, _Context>;
_ContextMap* g_examples{};

bool register_example(strv_t name, strv_t alias, strv_t doc, ExampleClosure&& closure) {
  if (!g_examples)
    g_examples = new _ContextMap;

  g_examples->insert(std::make_pair(alias, std::make_tuple(name, doc, std::move(closure))));
  return true;
}

int launch_examples(strv_t name) {
  if (!g_examples || g_examples->empty())
    return 0;

  if (!name.empty()) {
    if (name.compare("help") == 0) {
      std::cout << "USAGE: " << std::endl;
      for (auto& e : *g_examples)
        std::cout << std::left << std::setw(8) << e.first << "- " << std::get<1>(e.second) << std::endl;
    }
    else {
      if (auto it = g_examples->find(name); it != g_examples->end()) {
        std::cout
          << "========= [RUN EXAMPLE: "
          << std::get<0>(it->second)
          << "("
          << it->first
          << ")] ========="
          << std::endl;
        std::get<2>(it->second)();
      }
    }
  }
  else {
    for (auto& e : *g_examples) {
      std::cout
        << "========= [RUN EXAMPLE: "
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

COEXT_EXAMPLE(DummyExample, de, "A dummy example ...") {
}