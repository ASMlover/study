#include "runtime/core.hh"

#include "runtime/vm.hh"

namespace ms {

int RunScript(const std::string& source, std::string* error) {
  Vm vm;
  const InterpretResult r = vm.ExecuteSource(source, error);
  return static_cast<int>(r);
}

}  // namespace ms
