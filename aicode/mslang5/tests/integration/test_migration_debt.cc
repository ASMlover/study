#include "test_common.hh"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

struct DebtCase {
  const char* name;
  const char* script_path;
  ms::InterpretResult expected_result;
  const char* expected_error_code;
};

}  // namespace

int RunMigrationDebtTests() {
  const std::vector<DebtCase> debt_cases = {
      {"class_fields", "tests/scripts/language/class_fields.ms", ms::InterpretResult::kOk, ""},
      {"class_inherit", "tests/scripts/language/class_inherit.ms", ms::InterpretResult::kOk, ""},
            {"resolver_ok_this_in_nested_function",
       "tests/scripts/language/resolver_ok_this_in_nested_function.ms", ms::InterpretResult::kOk, ""},
      {"resolver_ok_super_in_subclass", "tests/scripts/language/resolver_ok_super_in_subclass.ms",
       ms::InterpretResult::kOk, ""},
      {"error_runtime_top_level_return", "tests/scripts/language/error_runtime_top_level_return.ms",
       ms::InterpretResult::kCompileError, "MS3001"},
      {"error_resolve_this_in_free_function",
       "tests/scripts/language/error_resolve_this_in_free_function.ms",
       ms::InterpretResult::kCompileError, "MS3002"},
      {"error_resolve_super_without_superclass_method",
       "tests/scripts/language/error_resolve_super_without_superclass_method.ms",
       ms::InterpretResult::kCompileError, "MS3003"},
      {"error_parse_self_inherit", "tests/scripts/language/error_parse_self_inherit.ms",
       ms::InterpretResult::kCompileError, "MS3004"},
  };

  const std::string repo_root = RepoRoot() + "/";
  int fallback_cases = 0;
  for (const auto& debt_case : debt_cases) {
    const std::string source = ReadAll(repo_root + debt_case.script_path);
    const ExecOutcome default_run =
        RunWithMode(source, ms::SourceExecutionMode::kVmPreferredWithLegacyFallback);
    const ExecOutcome legacy_run = RunWithMode(source, ms::SourceExecutionMode::kLegacyOnly);

    const std::string case_name = debt_case.name;
    Expect(default_run.result == debt_case.expected_result,
           case_name + " should keep expected result contract in debt suite");
    Expect(default_run.result == legacy_run.result,
           case_name + " should keep route-independent result while under debt tracking");
    if (std::string(debt_case.expected_error_code).empty()) {
      Expect(default_run.error.empty(), case_name + " should not produce diagnostics");
    } else {
      Expect(default_run.error.find(debt_case.expected_error_code) != std::string::npos,
             case_name + " should expose expected diagnostic code while under debt tracking");
    }
    Expect(default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy,
           case_name + " should stay classified as VM fallback migration debt");
    if (default_run.route == ms::SourceExecutionRoute::kVmCompileFailedThenLegacy) {
      ++fallback_cases;
    }
  }

  const double fallback_rate = debt_cases.empty()
                                   ? 0.0
                                   : static_cast<double>(fallback_cases) /
                                         static_cast<double>(debt_cases.size());
  std::cout << "[migration_debt] tracked_cases=" << debt_cases.size()
            << " fallback_cases=" << fallback_cases << " fallback_rate=" << std::fixed
            << std::setprecision(3) << fallback_rate << "\n";
  return 0;
}