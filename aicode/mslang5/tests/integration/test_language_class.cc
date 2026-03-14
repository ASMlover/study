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

ExecOutcome RunWithMode(const std::string& src) {
  ms::Vm vm;
  std::ostringstream out;
  vm.set_output(out);

  ExecOutcome outcome{ms::InterpretResult::kRuntimeError, "", "", ms::SourceExecutionRoute::kVmPipeline};
  outcome.result = vm.execute_source(src, &outcome.error);
  outcome.output = out.str();
  outcome.route = vm.last_source_execution_route();
  return outcome;
}

}  // namespace

int RunClassIntegrationTests() {
  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_fields.ms");
    const ExecOutcome default_run = RunWithMode(src);
    Expect(default_run.result == ms::InterpretResult::kOk, "class_fields should execute");
    Expect(default_run.output == "7\n9\n", "class_fields output should be 7,9");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "class_fields should execute directly on VM pipeline");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_inherit.ms");
    const ExecOutcome default_run = RunWithMode(src);
    Expect(default_run.result == ms::InterpretResult::kOk, "class_inherit should execute");
    Expect(default_run.output == "AB\n", "class_inherit output should be AB");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "class_inherit should execute directly on VM pipeline");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_super_error.ms");
    const ExecOutcome default_run = RunWithMode(src);
    Expect(default_run.result == ms::InterpretResult::kRuntimeError, "class_super_error should fail");
    Expect(default_run.error.find("MS4004") != std::string::npos,
           "missing super method should expose MS4004");
    Expect(default_run.error.find("undefined property: nope") != std::string::npos,
           "missing super method should report undefined property");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "class_super_error should execute via VM pipeline");
  }
  return 0;
}
