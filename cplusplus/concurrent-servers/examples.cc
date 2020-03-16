#include <iostream>
#include <memory>
#include <vector>
#include <tuple>
#include "examples.hh"

namespace efw {

using _Context = std::tuple<std::string_view, ClosureFn>;
using _ContextVector = std::vector<_Context>;

std::unique_ptr<_ContextVector> g_examples{};

bool register_example(std::string_view name, ClosureFn&& closure) {
  if (!g_examples)
    g_examples.reset(new _ContextVector);

  g_examples->push_back(std::make_tuple(name, std::move(closure)));
  return true;
}

int launch_all_examples() {
  if (!g_examples || g_examples->empty())
    return 0;

  for (auto& ex : *g_examples) {
    std::cout << "========= [RUN EXAMPLE - " << std::get<std::string_view>(ex) << "] =========" << std::endl;
    std::get<ClosureFn>(ex)();
  }

  return 0;
}

}

EFW_EXAMPLE(DummyWorker) {
}