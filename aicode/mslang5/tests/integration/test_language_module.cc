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

struct ExecOutcome {
  ms::InterpretResult result;
  std::string output;
  std::string error;
  ms::SourceExecutionRoute route;
};

ExecOutcome RunWithMode(const std::string& src, const ms::SourceExecutionMode mode) {
  ms::Vm vm;
  vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
  std::ostringstream out;
  vm.set_output(out);
  vm.set_source_execution_mode(mode);

  ExecOutcome outcome{ms::InterpretResult::kRuntimeError, "", "", ms::SourceExecutionRoute::kNone};
  outcome.result = vm.execute_source(src, &outcome.error);
  outcome.output = out.str();
  outcome.route = vm.last_source_execution_route();
  return outcome;
}

}  // namespace

int RunModuleIntegrationTests() {
  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/module/import_cache_and_alias.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferred);
    Expect(default_run.result == ms::InterpretResult::kOk, "import_cache_and_alias should execute");
    Expect(default_run.output == "init\n7\nok\n",
           "import_cache_and_alias output should show one-time init and alias binds");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "import_cache_and_alias should execute directly on VM pipeline");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/module/import_dotted_name.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferred);
    Expect(default_run.result == ms::InterpretResult::kOk, "import_dotted_name should execute");
    Expect(default_run.output == "42\n", "import_dotted_name output should be 42");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "import_dotted_name should execute directly on VM pipeline");
  }

  {
    const std::string src = "import side;\nprint x;\n";
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferred);
    Expect(default_run.result == ms::InterpretResult::kRuntimeError,
           "module declarations should not leak into importer globals");
    Expect(default_run.error.find("MS4001") != std::string::npos,
           "module symbol leakage should report MS4001 undefined variable");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "module scope visibility failure should execute on VM pipeline");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/module/error_missing_module.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferred);
    Expect(default_run.result == ms::InterpretResult::kRuntimeError, "missing module should fail");
    Expect(default_run.error.find("MS5001") != std::string::npos,
           "missing module should expose MS5001");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "missing module should execute on VM pipeline");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/module/error_missing_symbol.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferred);
    Expect(default_run.result == ms::InterpretResult::kRuntimeError, "missing symbol should fail");
    Expect(default_run.error.find("MS5002") != std::string::npos,
           "missing symbol should expose MS5002");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "missing symbol should execute on VM pipeline");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/module/error_cycle_entry.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferred);
    Expect(default_run.result == ms::InterpretResult::kRuntimeError, "module cycle should fail");
    Expect(default_run.error.find("MS5003") != std::string::npos,
           "module cycle should expose MS5003");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "module cycle should execute on VM pipeline");
  }

  {
    ms::Vm vm;
    vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");

    std::string first_error;
    const ms::InterpretResult first =
        vm.execute_source("import error_cycle_entry;\n", &first_error);
    Expect(first == ms::InterpretResult::kRuntimeError,
           "failed module fixture should fail first initialization");
    Expect(first_error.find("MS5003") != std::string::npos,
           "failed module fixture first error should expose MS5003");

    std::string second_error;
    const ms::InterpretResult second =
        vm.execute_source("import error_cycle_entry;\n", &second_error);
    Expect(second == ms::InterpretResult::kRuntimeError,
           "failed module fixture should fail on repeated initialization");
    Expect(second_error.find("MS5004") != std::string::npos,
           "failed module fixture should expose MS5004 from failed cache");
    Expect(vm.last_source_execution_route() == ms::SourceExecutionRoute::kVmPipeline,
           "failed-cache retry should execute on VM pipeline");
  }

  return 0;
}
