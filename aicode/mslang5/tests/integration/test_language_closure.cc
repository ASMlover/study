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
  std::ostringstream out;
  vm.SetOutput(out);
  vm.SetSourceExecutionMode(mode);

  ExecOutcome outcome{ms::InterpretResult::kRuntimeError, "", "", ms::SourceExecutionRoute::kNone};
  outcome.result = vm.ExecuteSource(src, &outcome.error);
  outcome.output = out.str();
  outcome.route = vm.LastSourceExecutionRoute();
  return outcome;
}

}  // namespace

int RunClosureIntegrationTests() {
  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_capture.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferredWithLegacyFallback);
    const ExecOutcome legacy_run = RunWithMode(src, ms::SourceExecutionMode::kLegacyOnly);
    Expect(default_run.result == ms::InterpretResult::kOk, "closure_capture should execute");
    Expect(default_run.output == "11\n12\n", "closure_capture output should be 11,12");
    Expect(default_run.output == legacy_run.output, "closure_capture output should be route independent");
    Expect(default_run.result == legacy_run.result, "closure_capture result should be route independent");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy,
           "closure_capture should use compatibility fallback until VM parity is complete");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_lexical.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferredWithLegacyFallback);
    const ExecOutcome legacy_run = RunWithMode(src, ms::SourceExecutionMode::kLegacyOnly);
    Expect(default_run.result == ms::InterpretResult::kOk, "closure_lexical should execute");
    Expect(default_run.output == "local\nglobal\n",
           "closure_lexical output should preserve lexical scope");
    Expect(default_run.output == legacy_run.output, "closure_lexical output should be route independent");
    Expect(default_run.result == legacy_run.result, "closure_lexical result should be route independent");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy,
           "closure_lexical should use compatibility fallback until VM parity is complete");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_arity_error.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferredWithLegacyFallback);
    const ExecOutcome legacy_run = RunWithMode(src, ms::SourceExecutionMode::kLegacyOnly);
    Expect(default_run.result == ms::InterpretResult::kRuntimeError, "closure_arity_error should fail");
    Expect(default_run.error.find("MS4002") != std::string::npos,
           "arity mismatch should expose MS4002");
    Expect(default_run.error.find("expected 2 arguments but got 1") != std::string::npos,
           "arity mismatch should report actual and expected arguments");
    Expect(default_run.error == legacy_run.error, "closure_arity_error message should be route independent");
    Expect(default_run.result == legacy_run.result, "closure_arity_error result should be route independent");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline ||
               default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy,
           "closure_arity_error should execute via VM pipeline or compatibility fallback");
  }
  return 0;
}
