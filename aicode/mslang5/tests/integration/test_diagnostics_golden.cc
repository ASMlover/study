#include "test_common.hh"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "frontend/parser.hh"
#include "runtime/vm.hh"

namespace {

struct ExpectedDiagnostic {
  std::string phase;
  std::string code;
  std::string file;
  std::size_t line = 1;
};
struct PrecisionCase {
  std::string id;
  std::string source;
  std::string phase;
  std::string code;
  std::size_t line = 1;
};

std::string NormalizePath(std::string path) {
  std::replace(path.begin(), path.end(), '\\', '/');
  return path;
}

std::string NormalizePhase(std::string phase) {
  std::transform(phase.begin(), phase.end(), phase.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return phase;
}

std::string NormalizeCode(std::string code) {
  std::transform(code.begin(), code.end(), code.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
  return code;
}

std::string ReadAll(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

std::optional<std::string> ExtractString(const std::string& text, const std::string& key) {
  const std::regex pattern("\"" + key + R"(\"\s*:\s*\"([^\"]*)\")");
  std::smatch match;
  if (std::regex_search(text, match, pattern)) {
    return match[1].str();
  }
  return std::nullopt;
}

std::optional<std::size_t> ExtractSize(const std::string& text, const std::string& key) {
  const std::regex pattern("\"" + key + R"(\"\s*:\s*([0-9]+))");
  std::smatch match;
  if (std::regex_search(text, match, pattern)) {
    return static_cast<std::size_t>(std::stoul(match[1].str()));
  }
  return std::nullopt;
}

std::optional<ExpectedDiagnostic> LoadExpected(const std::string& golden_path) {
  const std::string text = ReadAll(golden_path);
  ExpectedDiagnostic out;
  auto phase = ExtractString(text, "phase");
  auto code = ExtractString(text, "code");
  auto file = ExtractString(text, "file");
  auto line = ExtractSize(text, "line");
  if (!phase.has_value() || !code.has_value() || !file.has_value() || !line.has_value()) {
    Expect(false, "invalid diagnostics golden json: " + golden_path);
    return std::nullopt;
  }
  out.phase = NormalizePhase(*phase);
  out.code = NormalizeCode(*code);
  out.file = NormalizePath(*file);
  out.line = *line;
  return out;
}

void RunGoldenCase(const std::string& id, const std::string& script_path,
                   const ms::InterpretResult expected_result,
                   const std::string& golden_path) {
  auto expected = LoadExpected(golden_path);
  if (!expected.has_value()) {
    return;
  }

  const std::string source = ReadAll(script_path);
  ms::Vm vm;
  vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
  std::ostringstream output;
  vm.set_output(output);

  std::string error;
  const ms::InterpretResult result =
      vm.execute_source_named(source, NormalizePath(expected->file), &error);
  Expect(result == expected_result, id + " result mismatch");
  const auto& diagnostics = vm.last_diagnostics();
  Expect(!diagnostics.empty(), id + " should expose diagnostics");
  if (diagnostics.empty()) {
    return;
  }

  const auto& diag = diagnostics.front();
  Expect(NormalizePhase(diag.phase) == expected->phase, id + " phase mismatch");
  Expect(NormalizeCode(diag.code) == expected->code, id + " code mismatch");
  Expect(NormalizePath(diag.span.file) == expected->file, id + " file mismatch");
  Expect(diag.span.line == expected->line, id + " line mismatch");
}

void RunParserCascadeBoundCase() {
  const std::string id = "DIAG-PARSE-RECOVERY-BOUND-001";
  const std::string script_path = RepoRoot() + "/tests/scripts/diagnostics/parse_recovery_cascade_bound.ms";
  const std::string source = ReadAll(script_path);

  ms::Vm first_vm;
  first_vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
  std::ostringstream first_output;
  first_vm.set_output(first_output);
  std::string first_error;
  const ms::InterpretResult first_result =
      first_vm.execute_source_named(source, "tests/scripts/diagnostics/parse_recovery_cascade_bound.ms",
                                    &first_error);
  Expect(first_result == ms::InterpretResult::kCompileError, id + " should fail compile on first run");

  const auto& first_diagnostics = first_vm.last_diagnostics();
  Expect(!first_diagnostics.empty(), id + " should expose diagnostics on first run");
  Expect(first_diagnostics.size() <= ms::Parser::kMaxErrorCascadeCount,
         id + " should cap parse cascade count");

  ms::Vm second_vm;
  second_vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
  std::ostringstream second_output;
  second_vm.set_output(second_output);
  std::string second_error;
  const ms::InterpretResult second_result =
      second_vm.execute_source_named(source, "tests/scripts/diagnostics/parse_recovery_cascade_bound.ms",
                                     &second_error);
  Expect(second_result == ms::InterpretResult::kCompileError,
         id + " should fail compile on second run");

  const auto& second_diagnostics = second_vm.last_diagnostics();
  Expect(!second_diagnostics.empty(), id + " should expose diagnostics on second run");
  Expect(second_diagnostics.size() <= ms::Parser::kMaxErrorCascadeCount,
         id + " should cap parse cascade count on second run");

  if (first_diagnostics.empty() || second_diagnostics.empty()) {
    return;
  }

  const auto& first = first_diagnostics.front();
  const auto& second = second_diagnostics.front();
  Expect(NormalizePhase(first.phase) == NormalizePhase(second.phase),
         id + " should preserve first diagnostic phase across runs");
  Expect(NormalizeCode(first.code) == NormalizeCode(second.code),
         id + " should preserve first diagnostic code across runs");
  Expect(NormalizePath(first.span.file) == NormalizePath(second.span.file),
         id + " should preserve first diagnostic file across runs");
  Expect(first.span.line == second.span.line,
         id + " should preserve first diagnostic line across runs");
}

void RunRuntimeModuleSpanPrecisionCoverageCase() {
  const std::vector<PrecisionCase> cases = {
      {"DIAG-RUNTIME-SPAN-001", "\n\nprint missing_value\n", "runtime", "MS4001", 3},
      {"DIAG-RUNTIME-SPAN-002", "fun add(a, b) {\n  return a + b\n}\n\nprint add(1)\n", "runtime",
       "MS4002", 5},
      {"DIAG-RUNTIME-SPAN-003", "class Foo {}\nvar f = Foo()\nprint f.nope\n", "runtime", "MS4004",
       3},
      {"DIAG-RUNTIME-SPAN-004", "var n = 42\nn()\n", "runtime", "MS4005", 2},
      {"DIAG-RUNTIME-SPAN-005", "import std.str\nprint str.len(123, 456)\n", "runtime", "MS4002",
       2},
      {"DIAG-MODULE-SPAN-001", "\n\nimport no_such_module\n", "module", "MS5001", 4},
      {"DIAG-MODULE-SPAN-002", "\nfrom util import not_exported as alias\n", "module", "MS5002", 4},
      {"DIAG-MODULE-SPAN-003", "\n\n\nimport cycle_a\n", "module", "MS5003", 2},
      {"DIAG-MODULE-SPAN-004", "import fail_init_runtime\n", "module", "MS5004", 2},
  };

  std::size_t precise_count = 0;
  for (const auto& test_case : cases) {
    ms::Vm vm;
    vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
    std::ostringstream output;
    vm.set_output(output);

    std::string error;
    const ms::InterpretResult result =
        vm.execute_source_named(test_case.source, "tests/scripts/diagnostics/runtime_span_precision.ms",
                                &error);
    Expect(result == ms::InterpretResult::kRuntimeError, test_case.id + " should fail at runtime");

    const auto& diagnostics = vm.last_diagnostics();
    Expect(!diagnostics.empty(), test_case.id + " should expose diagnostics");
    if (diagnostics.empty()) {
      continue;
    }

    const auto& diagnostic = diagnostics.front();
    const bool phase_matches = NormalizePhase(diagnostic.phase) == NormalizePhase(test_case.phase);
    const bool code_matches = NormalizeCode(diagnostic.code) == NormalizeCode(test_case.code);
    const bool line_matches = diagnostic.span.line == test_case.line;
    const bool precise = phase_matches && code_matches && line_matches;
    if (precise) {
      ++precise_count;
      continue;
    }

    std::ostringstream mismatch;
    mismatch << test_case.id << " mismatch: expected " << test_case.phase << "/" << test_case.code
             << " line " << test_case.line << ", got " << diagnostic.phase << "/"
             << diagnostic.code << " line " << diagnostic.span.line;
    Expect(false, mismatch.str());
  }

  const double precision_ratio =
      cases.empty() ? 1.0 : static_cast<double>(precise_count) / static_cast<double>(cases.size());
  std::ostringstream ratio_message;
  ratio_message << "runtime/module span precision ratio should be >= 0.95, got "
                << precision_ratio;
  Expect(precision_ratio >= 0.95, ratio_message.str());
}
}  // namespace

int RunDiagnosticsGoldenTests() {
  RunGoldenCase("DIAG-PARSE-EXPR-001",
                RepoRoot() + "/tests/scripts/diagnostics/parse_expected_expression.ms",
                ms::InterpretResult::kCompileError,
                RepoRoot() + "/tests/diagnostics/samples/parse_expected_expression.golden.json");
  RunGoldenCase("DIAG-RESOLVE-RETURN-001",
                RepoRoot() + "/tests/scripts/diagnostics/resolve_return_outside_function.ms",
                ms::InterpretResult::kCompileError,
                RepoRoot() +
                    "/tests/diagnostics/samples/resolve_return_outside_function.golden.json");
  RunGoldenCase("DIAG-RUN-ARITY-001",
                RepoRoot() + "/tests/scripts/diagnostics/runtime_arity_mismatch.ms",
                ms::InterpretResult::kRuntimeError,
                RepoRoot() + "/tests/diagnostics/samples/runtime_arity_mismatch.golden.json");
  RunGoldenCase("DIAG-MOD-MISSING-001",
                RepoRoot() + "/tests/scripts/diagnostics/module_not_found.ms",
                ms::InterpretResult::kRuntimeError,
                RepoRoot() + "/tests/diagnostics/samples/module_not_found.golden.json");
  RunGoldenCase("DIAG-PARSE-RECOVERY-BOUND-001",
                RepoRoot() + "/tests/scripts/diagnostics/parse_recovery_cascade_bound.ms",
                ms::InterpretResult::kCompileError,
                RepoRoot() +
                    "/tests/diagnostics/samples/parse_recovery_cascade_bound.golden.json");
  RunParserCascadeBoundCase();
  RunRuntimeModuleSpanPrecisionCoverageCase();
  return 0;
}
