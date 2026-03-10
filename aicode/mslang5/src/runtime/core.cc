#include "runtime/core.hh"

#include "runtime/vm.hh"

namespace ms {

int run_script(const std::string& source, std::string* error) {
  Vm vm;
  const InterpretResult r = vm.execute_source(source, error);
  return static_cast<int>(r);
}

}  // namespace ms
