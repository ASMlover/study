#include <iostream>
#include <memory>
#include <vector>
#include <tuple>
#include <unordered_map>
#include "examples.hh"

namespace efw {

using _ContextDict = std::unordered_map<std::string_view, std::tuple<std::string_view, ClosureFn>>;
_ContextDict* g_examples{};

bool register_example(std::string_view name, std::string_view alias, ClosureFn&& closure) {
  if (!g_examples)
    g_examples = new _ContextDict;

  g_examples->insert(std::make_pair(alias, std::make_tuple(name, std::move(closure))));
  return true;
}

int launch_examples(std::string_view name) {
  if (!g_examples || g_examples->empty())
    return 0;

  if (!name.empty()) {
    if (auto it = g_examples->find(name); it != g_examples->end()) {
      std::cout << "========= [RUN EXAMPLE - " << it->first << ":" << std::get<0>(it->second) << "] =========" << std::endl;
      std::get<1>(it->second)();
    }
  }
  else {
    for (auto& ex : *g_examples) {
      std::cout << "========= [RUN EXAMPLE - " << ex.first << ":" << std::get<0>(ex.second) << "] =========" << std::endl;
      std::get<1>(ex.second)();
    }
  }

  return 0;
}

}

EFW_EXAMPLE(DummyWorker, dw) {
}