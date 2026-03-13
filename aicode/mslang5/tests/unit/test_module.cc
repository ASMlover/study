#include "test_common.hh"

#include <fstream>
#include <sstream>
#include <string>

#include "runtime/vm.hh"

namespace {

std::string ReadAll(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

}  // namespace

int RunModuleTests() {
  ms::Vm vm;
  vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
  std::ostringstream out;
  vm.set_output(out);

  std::string error;
  ms::InterpretResult r = vm.execute_source(
      "import side;\nimport side;\nfrom util import value as v;\nprint v;\n", &error);
  Expect(r == ms::InterpretResult::kOk, "module script should run");
  const std::string text = out.str();
  const std::size_t first = text.find("init");
  const std::size_t second = text.find("init", first == std::string::npos ? 0 : first + 1);
  Expect(first != std::string::npos, "side effect should run once");
  Expect(second == std::string::npos, "cached import should avoid double init");
  Expect(text.find("7") != std::string::npos, "from-import alias should bind symbol");

  std::string missing_error;
  r = vm.execute_source("import no_such_module;\n", &missing_error);
  Expect(r == ms::InterpretResult::kRuntimeError, "missing module should fail");
  Expect(missing_error.find("MS5001") != std::string::npos,
         "missing module should expose MS5001");
  Expect(missing_error.find("module not found") != std::string::npos,
         "missing module should report clear error");

  std::string missing_symbol_error;
  r = vm.execute_source("from util import not_exported as x;\n", &missing_symbol_error);
  Expect(r == ms::InterpretResult::kRuntimeError, "missing module symbol should fail");
  Expect(missing_symbol_error.find("MS5002") != std::string::npos,
         "missing module symbol should expose MS5002");

  std::string cycle_error;
  r = vm.execute_source(ReadAll(RepoRoot() + "/tests/scripts/module/error_cycle_entry.ms"),
                       &cycle_error);
  Expect(r == ms::InterpretResult::kRuntimeError, "cycle import should fail");
  Expect(cycle_error.find("MS5003") != std::string::npos, "cycle import should expose MS5003");

  std::string scope_error;
  r = vm.execute_source("import side;\nprint x;\n", &scope_error);
  Expect(r == ms::InterpretResult::kRuntimeError,
         "module declarations should not leak into importer globals");
  Expect(scope_error.find("MS4001") != std::string::npos,
         "module scope isolation should expose MS4001 for leaked symbol access");

  std::string failed_first_error;
  r = vm.execute_source("import error_cycle_entry;\n", &failed_first_error);
  Expect(r == ms::InterpretResult::kRuntimeError,
         "failed module fixture should fail first initialization");
  Expect(failed_first_error.find("MS5003") != std::string::npos,
         "failed module fixture first error should expose MS5003");

  std::string failed_retry_error;
  r = vm.execute_source("import error_cycle_entry;\n", &failed_retry_error);
  Expect(r == ms::InterpretResult::kRuntimeError,
         "failed module fixture should fail on repeated initialization");
  Expect(failed_retry_error.find("MS5004") != std::string::npos,
         "failed module fixture retry should expose MS5004");
  return 0;
}
