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

int RunClosureIntegrationTests() {
  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_capture.ms");
    const ExecOutcome default_run = RunWithMode(src);
    Expect(default_run.result == ms::InterpretResult::kOk, "closure_capture should execute");
    Expect(default_run.output == "11\n12\n", "closure_capture output should be 11,12");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "closure_capture should execute on VM pipeline");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_lexical.ms");
    const ExecOutcome default_run = RunWithMode(src);
    Expect(default_run.result == ms::InterpretResult::kOk, "closure_lexical should execute");
    Expect(default_run.output == "local\nglobal\n",
           "closure_lexical output should preserve lexical scope");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "closure_lexical should execute on VM pipeline");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_arity_error.ms");
    const ExecOutcome default_run = RunWithMode(src);
    Expect(default_run.result == ms::InterpretResult::kRuntimeError, "closure_arity_error should fail");
    Expect(default_run.error.find("MS4002") != std::string::npos,
           "arity mismatch should expose MS4002");
    Expect(default_run.error.find("expected 2 arguments but got 1") != std::string::npos,
           "arity mismatch should report actual and expected arguments");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline,
           "closure_arity_error should execute on VM pipeline");
  }
  return 0;
}
