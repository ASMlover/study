#include "test_common.hh"

#include <sstream>
#include <string>

#include "runtime/vm.hh"

int RunModuleTests() {
  ms::Vm vm;
  vm.Modules().AddSearchPath(RepoRoot() + "/tests/scripts/module");
  std::ostringstream out;
  vm.SetOutput(out);

  std::string error;
  ms::InterpretResult r = vm.ExecuteSource(
      "import side;\nimport side;\nfrom util import value as v;\nprint v;\n", &error);
  Expect(r == ms::InterpretResult::kOk, "module script should run");
  const std::string text = out.str();
  const std::size_t first = text.find("init");
  const std::size_t second = text.find("init", first == std::string::npos ? 0 : first + 1);
  Expect(first != std::string::npos, "side effect should run once");
  Expect(second == std::string::npos, "cached import should avoid double init");
  Expect(text.find("7") != std::string::npos, "from-import alias should bind symbol");

  std::string missing_error;
  r = vm.ExecuteSource("import no_such_module;\n", &missing_error);
  Expect(r == ms::InterpretResult::kRuntimeError, "missing module should fail");
  Expect(missing_error.find("module not found") != std::string::npos,
         "missing module should report clear error");
  return 0;
}
