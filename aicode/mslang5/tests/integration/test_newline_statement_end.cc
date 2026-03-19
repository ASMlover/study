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

int RunNewlineStatementEndIntegrationTests() {
  {
    const std::string src =
        ReadAll(RepoRoot() + "/tests/scripts/migration/newline/newline_statement_end_ok.ms");
    const ExecOutcome run = RunWithMode(src);
    Expect(run.result == ms::InterpretResult::kOk,
           "newline_statement_end_ok should execute");
    Expect(run.output == "4\n9\n9\n",
           "newline_statement_end_ok output should match expected values");
    Expect(run.route == ms::SourceExecutionRoute::kVmPipeline,
           "newline_statement_end_ok should execute on VM pipeline");
  }

  {
    const std::string src =
        ReadAll(RepoRoot() + "/tests/scripts/migration/newline/newline_semicolon_mixed_ok.ms");
    const ExecOutcome run = RunWithMode(src);
    Expect(run.result == ms::InterpretResult::kOk,
           "newline_semicolon_mixed_ok should execute");
    Expect(run.output == "3\n6\n",
           "newline_semicolon_mixed_ok output should match expected values");
    Expect(run.route == ms::SourceExecutionRoute::kVmPipeline,
           "newline_semicolon_mixed_ok should execute on VM pipeline");
  }

  {
    const ExecOutcome run = RunWithMode("return\n");
    Expect(run.result == ms::InterpretResult::kCompileError,
           "top-level return with newline terminator should remain compile error");
    Expect(run.error.find("MS3001") != std::string::npos,
           "top-level return newline path should preserve MS3001 diagnostic");
  }

  return 0;
}