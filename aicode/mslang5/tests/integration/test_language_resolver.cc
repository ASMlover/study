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
  vm.set_output(out);
  vm.set_source_execution_mode(mode);

  ExecOutcome outcome{ms::InterpretResult::kRuntimeError, "", "", ms::SourceExecutionRoute::kNone};
  outcome.result = vm.execute_source(src, &outcome.error);
  outcome.output = out.str();
  outcome.route = vm.last_source_execution_route();
  return outcome;
}

void ExpectResolveCompileFailure(const std::string& script_path, const std::string& code) {
  const std::string src = ReadAll(script_path);
  const ExecOutcome run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferred);

  Expect(run.result == ms::InterpretResult::kCompileError,
         "resolver violation should classify as compile error");
  Expect(run.error.find("[resolve ") != std::string::npos,
         "resolver violation should report resolve-phase diagnostics");
  Expect(run.error.find(code) != std::string::npos, "resolver violation should expose MS3xxx code");
  Expect(run.output.empty(), "resolver failures should not produce output");
  Expect(run.route == ms::SourceExecutionRoute::kVmPipeline,
         "resolver script should run on VM pipeline");
}

void ExpectResolverSuccess(const std::string& script_path, const std::string& expected_output) {
  const std::string src = ReadAll(script_path);
  const ExecOutcome run = RunWithMode(src, ms::SourceExecutionMode::kVmPreferred);

  Expect(run.result == ms::InterpretResult::kOk, "resolver success script should execute");
  Expect(run.output == expected_output, "resolver success script output should match expectation");
  Expect(run.error.empty(), "resolver success script should not produce errors");
  Expect(run.route == ms::SourceExecutionRoute::kVmPipeline,
         "resolver success scripts should run on VM pipeline");
}

}  // namespace

int RunResolverIntegrationTests() {
  const std::string base = RepoRoot() + "/tests/scripts/language/";
  ExpectResolverSuccess(base + "resolver_ok_return_in_function.ms", "42\n");
  ExpectResolverSuccess(base + "resolver_ok_this_in_nested_function.ms", "9\n");
  ExpectResolverSuccess(base + "resolver_ok_super_in_subclass.ms", "base-mid-leaf\n");

  ExpectResolveCompileFailure(base + "error_runtime_top_level_return.ms", "MS3001");
  ExpectResolveCompileFailure(base + "error_resolve_top_level_return_in_block.ms", "MS3001");
  ExpectResolveCompileFailure(base + "error_runtime_this_outside_class.ms", "MS3002");
  ExpectResolveCompileFailure(base + "error_resolve_this_in_free_function.ms", "MS3002");
  ExpectResolveCompileFailure(base + "error_runtime_super_outside_subclass.ms", "MS3003");
  ExpectResolveCompileFailure(base + "error_resolve_super_without_superclass_method.ms", "MS3003");
  ExpectResolveCompileFailure(base + "error_parse_self_inherit.ms", "MS3004");
  return 0;
}
