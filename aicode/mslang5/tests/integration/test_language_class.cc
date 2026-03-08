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

int RunClassIntegrationTests() {
  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_fields.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferredWithLegacyFallback);
    const ExecOutcome legacy_run = RunWithMode(src, ms::SourceExecutionMode::kLegacyOnly);
    Expect(default_run.result == ms::InterpretResult::kOk, "class_fields should execute");
    Expect(default_run.output == "7\n9\n", "class_fields output should be 7,9");
    Expect(default_run.output == legacy_run.output, "class_fields output should be route independent");
    Expect(default_run.result == legacy_run.result, "class_fields result should be route independent");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy,
           "class_fields should use compatibility fallback until VM parity is complete");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_inherit.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferredWithLegacyFallback);
    const ExecOutcome legacy_run = RunWithMode(src, ms::SourceExecutionMode::kLegacyOnly);
    Expect(default_run.result == ms::InterpretResult::kOk, "class_inherit should execute");
    Expect(default_run.output == "AB\n", "class_inherit output should be AB");
    Expect(default_run.output == legacy_run.output, "class_inherit output should be route independent");
    Expect(default_run.result == legacy_run.result, "class_inherit result should be route independent");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy,
           "class_inherit should use compatibility fallback until VM parity is complete");
  }

  {
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_super_error.ms");
    const ExecOutcome default_run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferredWithLegacyFallback);
    const ExecOutcome legacy_run = RunWithMode(src, ms::SourceExecutionMode::kLegacyOnly);
    Expect(default_run.result == ms::InterpretResult::kRuntimeError, "class_super_error should fail");
    Expect(default_run.error.find("MS4004") != std::string::npos,
           "missing super method should expose MS4004");
    Expect(default_run.error.find("undefined property: nope") != std::string::npos,
           "missing super method should report undefined property");
    Expect(default_run.error == legacy_run.error, "class_super_error message should be route independent");
    Expect(default_run.result == legacy_run.result, "class_super_error result should be route independent");
    Expect(default_run.route == ms::SourceExecutionRoute::kVmPipeline ||
               default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy,
           "class_super_error should execute via VM pipeline or compatibility fallback");
  }
  return 0;
}
