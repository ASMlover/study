#include "test_common.hh"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "runtime/vm.hh"

namespace {

struct ConformanceCase {
  std::string path;
  std::string id;
  std::vector<std::string> specs;
  std::string expect;
  std::optional<std::string> diag_phase;
  std::optional<std::string> diag_code;
  std::optional<std::string> diag_message_contains;
  std::optional<std::string> stdout_exact;
  std::string source;
};

std::string Trim(const std::string& value) {
  std::size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
    ++start;
  }
  std::size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
    --end;
  }
  return value.substr(start, end - start);
}

std::string NormalizeNewline(std::string text) {
  text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
  return text;
}

std::string ReadAll(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

std::optional<ConformanceCase> ParseCase(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open()) {
    Expect(false, "failed to open conformance case: " + path.string());
    return std::nullopt;
  }

  ConformanceCase out;
  out.path = std::filesystem::relative(path, RepoRoot()).generic_string();
  std::ostringstream source_builder;
  std::string line;
  bool in_header = true;
  bool capture_stdout = false;
  std::ostringstream stdout_builder;
  while (std::getline(in, line)) {
    const std::string normalized = NormalizeNewline(line);
    if (in_header) {
      if (normalized.rfind("// @", 0) == 0) {
        const std::size_t colon = normalized.find(':');
        if (colon == std::string::npos) {
          Expect(false, "invalid metadata line: " + normalized);
          return std::nullopt;
        }
        const std::string key = Trim(normalized.substr(4, colon - 4));
        const std::string value = Trim(normalized.substr(colon + 1));
        capture_stdout = false;
        if (key == "id") {
          out.id = value;
        } else if (key == "spec") {
          out.specs.push_back(value);
        } else if (key == "expect") {
          out.expect = value;
        } else if (key == "diag.phase") {
          out.diag_phase = value;
        } else if (key == "diag.code") {
          out.diag_code = value;
        } else if (key == "diag.message_contains") {
          out.diag_message_contains = value;
        } else if (key == "stdout") {
          capture_stdout = true;
        }
        continue;
      }
      if (capture_stdout && normalized.rfind("//", 0) == 0) {
        std::string value = normalized.substr(2);
        if (!value.empty() && value[0] == ' ') {
          value.erase(value.begin());
        }
        stdout_builder << value << "\n";
        continue;
      }
      in_header = false;
    }
    source_builder << normalized << "\n";
  }

  out.source = source_builder.str();
  const std::string captured_stdout = stdout_builder.str();
  if (!captured_stdout.empty()) {
    out.stdout_exact = captured_stdout;
  }

  if (out.id.empty() || out.specs.empty() || out.expect.empty()) {
    Expect(false, "missing required metadata in case: " + out.path);
    return std::nullopt;
  }
  if ((out.expect == "compile_error" || out.expect == "runtime_error") &&
      (!out.diag_phase.has_value() || !out.diag_code.has_value())) {
    Expect(false, "missing @diag.phase/@diag.code for failing case: " + out.path);
    return std::nullopt;
  }
  return out;
}

std::vector<ConformanceCase> LoadAllCases() {
  std::vector<ConformanceCase> cases;
  const std::filesystem::path root = std::filesystem::path(RepoRoot()) / "tests" / "conformance";
  if (!std::filesystem::exists(root)) {
    Expect(false, "conformance root not found");
    return cases;
  }
  for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".ms") {
      continue;
    }
    auto parsed = ParseCase(entry.path());
    if (parsed.has_value()) {
      cases.push_back(std::move(*parsed));
    }
  }
  std::sort(cases.begin(), cases.end(),
            [](const ConformanceCase& a, const ConformanceCase& b) { return a.path < b.path; });
  return cases;
}

void RunConformanceCase(const ConformanceCase& c) {
  ms::Vm vm;
  std::ostringstream output;
  vm.SetOutput(output);
  vm.Modules().AddSearchPath(RepoRoot() + "/tests/scripts/module");

  std::string error;
  const ms::InterpretResult result = vm.ExecuteSourceNamed(c.source, c.path, &error);
  const std::string out = NormalizeNewline(output.str());

  if (c.expect == "ok") {
    Expect(result == ms::InterpretResult::kOk, c.id + " expected ok");
    if (c.stdout_exact.has_value()) {
      Expect(out == NormalizeNewline(*c.stdout_exact), c.id + " stdout mismatch");
    }
    return;
  }

  if (c.expect == "compile_error") {
    Expect(result == ms::InterpretResult::kCompileError, c.id + " expected compile_error");
  } else if (c.expect == "runtime_error") {
    Expect(result == ms::InterpretResult::kRuntimeError, c.id + " expected runtime_error");
  } else {
    Expect(false, c.id + " has unsupported @expect value");
    return;
  }

  const auto& diagnostics = vm.LastDiagnostics();
  Expect(!diagnostics.empty(), c.id + " should expose diagnostics");
  if (diagnostics.empty()) {
    return;
  }
  const auto& diag = diagnostics.front();
  Expect(diag.phase == *c.diag_phase, c.id + " diagnostic phase mismatch");
  if (diag.code != *c.diag_code) {
    Expect(false, c.id + " diagnostic code mismatch: expected=" + *c.diag_code +
                      " actual=" + diag.code + " error=" + error);
  }
  if (c.diag_message_contains.has_value()) {
    Expect(diag.message.find(*c.diag_message_contains) != std::string::npos,
           c.id + " diagnostic message mismatch");
  }
}

}  // namespace

int RunConformanceIntegrationTests() {
  const std::vector<ConformanceCase> cases = LoadAllCases();
  Expect(!cases.empty(), "conformance cases should not be empty");
  for (const auto& c : cases) {
    RunConformanceCase(c);
  }
  return 0;
}
