#include "test_common.hh"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>

#include "runtime/vm.hh"

namespace {

struct ExpectedDiagnostic {
  std::string phase;
  std::string code;
  std::string message;
  std::string file;
  std::size_t line = 1;
};

std::string NormalizePath(std::string path) {
  std::replace(path.begin(), path.end(), '\\', '/');
  return path;
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
  auto message = ExtractString(text, "message");
  auto file = ExtractString(text, "file");
  auto line = ExtractSize(text, "line");
  if (!phase.has_value() || !code.has_value() || !message.has_value() || !file.has_value() ||
      !line.has_value()) {
    Expect(false, "invalid diagnostics golden json: " + golden_path);
    return std::nullopt;
  }
  out.phase = *phase;
  out.code = *code;
  out.message = *message;
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
  Expect(diag.phase == expected->phase, id + " phase mismatch");
  Expect(diag.code == expected->code, id + " code mismatch");
  Expect(diag.message == expected->message, id + " message mismatch");
  Expect(NormalizePath(diag.span.file) == expected->file, id + " file mismatch");
  Expect(diag.span.line == expected->line, id + " line mismatch");
}

}  // namespace

int RunDiagnosticsGoldenTests() {
  RunGoldenCase("DIAG-RUN-ARITY-001", RepoRoot() + "/tests/scripts/language/closure_arity_error.ms",
                ms::InterpretResult::kRuntimeError,
                RepoRoot() + "/tests/diagnostics/samples/runtime_arity_mismatch.golden.json");
  RunGoldenCase("DIAG-MOD-MISSING-001", RepoRoot() + "/tests/scripts/module/error_missing_module.ms",
                ms::InterpretResult::kRuntimeError,
                RepoRoot() + "/tests/diagnostics/samples/module_not_found.golden.json");
  return 0;
}
