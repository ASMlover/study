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

void ExpectResolveCompileFailure(const std::string& script_path, const std::string& code) {
  const std::string src = ReadAll(script_path);
  const ExecOutcome default_run =
      RunWithMode(src, ms::SourceExecutionMode::kVmPreferredWithLegacyFallback);
  const ExecOutcome legacy_run = RunWithMode(src, ms::SourceExecutionMode::kLegacyOnly);

  Expect(default_run.result == ms::InterpretResult::kCompileError,
         "default route should classify resolver violation as compile error");
  Expect(legacy_run.result == ms::InterpretResult::kCompileError,
         "legacy route should classify resolver violation as compile error");
  Expect(default_run.error.find("resolve error") != std::string::npos,
         "resolver violation should report resolve-phase diagnostics");
  Expect(default_run.error.find(code) != std::string::npos, "resolver violation should expose MS3xxx code");
  Expect(default_run.error == legacy_run.error, "resolver diagnostics should be route independent");
  Expect(default_run.output.empty(), "resolver failures should not produce output");
  Expect(default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy,
         "resolver script should run on fallback path while VM frontend is partial");
  Expect(legacy_run.route == ms::SourceExecutionRoute::kLegacyInterpreter,
         "legacy-only mode should remain explicit");
}

}  // namespace

int RunResolverIntegrationTests() {
  const std::string base = RepoRoot() + "/tests/scripts/language/";
  ExpectResolveCompileFailure(base + "error_runtime_top_level_return.ms", "MS3001");
  ExpectResolveCompileFailure(base + "error_runtime_this_outside_class.ms", "MS3002");
  ExpectResolveCompileFailure(base + "error_runtime_super_outside_subclass.ms", "MS3003");
  ExpectResolveCompileFailure(base + "error_parse_self_inherit.ms", "MS3004");
  return 0;
}
