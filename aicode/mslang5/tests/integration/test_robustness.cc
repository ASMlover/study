#include "test_common.hh"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "bytecode/chunk.hh"
#include "bytecode/opcode.hh"
#include "frontend/parser.hh"
#include "runtime/vm.hh"

namespace {

struct CorpusEntry {
  std::string id;
  std::string path;
  std::string tier;
};

std::optional<CorpusEntry> ParseCorpusLine(const std::string& raw_line) {
  if (raw_line.empty() || raw_line[0] == '#') {
    return std::nullopt;
  }

  const std::size_t first = raw_line.find(',');
  const std::size_t second = raw_line.find(',', first == std::string::npos ? first : first + 1);
  if (first == std::string::npos || second == std::string::npos || second + 1 >= raw_line.size()) {
    Expect(false, "invalid robustness corpus line: " + raw_line);
    return std::nullopt;
  }

  CorpusEntry entry;
  entry.id = raw_line.substr(0, first);
  entry.path = raw_line.substr(first + 1, second - first - 1);
  entry.tier = raw_line.substr(second + 1);
  return entry;
}

std::vector<CorpusEntry> LoadCorpus() {
  const std::filesystem::path corpus_path =
      std::filesystem::path(RepoRoot()) / "tests/scripts/robustness/parser_fuzz_corpus.txt";

  std::ifstream in(corpus_path, std::ios::binary);
  Expect(in.good(), "robustness corpus file should be readable");
  if (!in.good()) {
    return {};
  }

  std::vector<CorpusEntry> entries;
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    auto entry = ParseCorpusLine(line);
    if (entry.has_value()) {
      entries.push_back(std::move(*entry));
    }
  }

  Expect(!entries.empty(), "robustness corpus should contain at least one seed");
  return entries;
}

std::string ReadAll(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

std::optional<std::string> ReadEnvVar(const char* name) {
#ifdef _WIN32
  char* buffer = nullptr;
  std::size_t size = 0;
  if (_dupenv_s(&buffer, &size, name) != 0 || buffer == nullptr || size == 0) {
    if (buffer != nullptr) {
      free(buffer);
    }
    return std::nullopt;
  }
  std::string value(buffer);
  free(buffer);
  return value;
#else
  const char* value = std::getenv(name);
  if (value == nullptr || value[0] == '\0') {
    return std::nullopt;
  }
  return std::string(value);
#endif
}

std::uint32_t ResolveNondeterministicSeed() {
  const auto seed_env = ReadEnvVar("MAPLE_ROBUSTNESS_SEED");
  if (!seed_env.has_value()) {
    return static_cast<std::uint32_t>(
        std::chrono::steady_clock::now().time_since_epoch().count() & 0xffffffffu);
  }

  try {
    return static_cast<std::uint32_t>(std::stoul(*seed_env));
  } catch (...) {
    Expect(false, "MAPLE_ROBUSTNESS_SEED should be an unsigned integer");
    return 0x5EED1234u;
  }
}

int ResolveNondeterministicRounds() {
  const auto rounds_env = ReadEnvVar("MAPLE_ROBUSTNESS_ROUNDS");
  if (!rounds_env.has_value()) {
    return 24;
  }

  try {
    const int parsed = std::stoi(*rounds_env);
    return std::clamp(parsed, 1, 200);
  } catch (...) {
    Expect(false, "MAPLE_ROBUSTNESS_ROUNDS should be an integer");
    return 24;
  }
}

void RunParserFuzzDeterministicCase() {
  const std::vector<CorpusEntry> corpus = LoadCorpus();
  if (corpus.empty()) {
    return;
  }

  std::size_t executed_cases = 0;
  for (const auto& entry : corpus) {
    if (entry.tier != "deterministic") {
      continue;
    }

    const std::filesystem::path source_path = std::filesystem::path(RepoRoot()) / entry.path;
    const std::string source = ReadAll(source_path);
    Expect(!source.empty(), entry.id + " deterministic parser-fuzz seed should be non-empty");

    ms::Vm vm;
    vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
    std::ostringstream output;
    vm.set_output(output);

    std::string error;
    const ms::InterpretResult result =
        vm.execute_source_named(source, "tests/scripts/robustness/parser_fuzz_seed.ms", &error);
    Expect(result == ms::InterpretResult::kCompileError,
           entry.id + " deterministic parser-fuzz seed should fail in compile stage");

    const auto& diagnostics = vm.last_diagnostics();
    Expect(!diagnostics.empty(),
           entry.id + " deterministic parser-fuzz seed should emit diagnostics");
    if (!diagnostics.empty()) {
      const std::string& phase = diagnostics.front().phase;
      const bool parse_scoped = (phase == "parse" || phase == "lex");
      Expect(parse_scoped,
             entry.id + " deterministic parser-fuzz seed should stay parse/lex scoped");
    }
    Expect(diagnostics.size() <= ms::Parser::kMaxErrorCascadeCount,
           entry.id + " deterministic parser-fuzz seed should respect cascade cap");
    ++executed_cases;
  }

  std::cout << "[robustness][deterministic] suite=parser_fuzz_seed_replay"
            << " executed_cases=" << executed_cases << "\n";
}

void RunBytecodeInvariantDeterministicCase() {
  {
    ms::Vm vm;
    ms::Chunk malformed;
    malformed.write_op(ms::OpCode::kConstant, 1);

    std::string error;
    const ms::InterpretResult result = vm.execute(malformed, &error);
    Expect(result == ms::InterpretResult::kRuntimeError,
           "bytecode invariant preflight should reject truncated constant operands");
    Expect(error.find("bytecode invariant violated") != std::string::npos,
           "bytecode invariant preflight should report violation context");
  }

  {
    ms::Vm vm;
    ms::Chunk malformed;
    const std::size_t index = malformed.add_constant(std::string("not_function_prototype"));
    malformed.write_op(ms::OpCode::kClosure, 1);
    malformed.write(static_cast<std::uint8_t>(index), 1);

    std::string error;
    const ms::InterpretResult result = vm.execute(malformed, &error);
    Expect(result == ms::InterpretResult::kRuntimeError,
           "bytecode invariant preflight should reject closure with non-prototype constant");
    Expect(error.find("bytecode invariant violated") != std::string::npos,
           "bytecode invariant preflight should keep shared failure prefix");
  }

  std::cout << "[robustness][deterministic] suite=bytecode_invariant status=ok\n";
}

void RunGcStressDeterministicCase() {
  const std::string stress_script =
      "class Box {\n"
      "  init(v) { this.value = v; }\n"
      "}\n"
      "fun churn(limit) {\n"
      "  var i = 0;\n"
      "  while (i < limit) {\n"
      "    var temp = Box(i);\n"
      "    i = i + 1;\n"
      "  }\n"
      "}\n"
      "var keep = Box(999);\n"
      "churn(240);\n"
      "print keep.value;\n";

  ms::Vm vm;
  vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
  std::ostringstream output;
  vm.set_output(output);
  vm.gc().set_threshold(128);

  std::string error;
  const ms::InterpretResult preload = vm.execute_source("import side;\n", &error);
  Expect(preload == ms::InterpretResult::kOk,
         "gc stress profile should preload module cache");

  constexpr int kRounds = 4;
  std::size_t min_live_bytes = std::numeric_limits<std::size_t>::max();
  std::size_t max_live_bytes = 0;

  for (int round = 0; round < kRounds; ++round) {
    const ms::GcStats before = vm.gc().stats();

    error.clear();
    const ms::InterpretResult result = vm.execute_source(stress_script, &error);
    Expect(result == ms::InterpretResult::kOk,
           "gc stress profile should execute without runtime failure");
    Expect(vm.last_source_execution_route() == ms::SourceExecutionRoute::kVmPipeline,
           "gc stress profile should remain on VM pipeline");

    const ms::GcStats after = vm.gc().stats();
    Expect(after.collections > before.collections,
           "gc stress profile should advance collection count each round");
    Expect(after.bytes_reclaimed > before.bytes_reclaimed,
           "gc stress profile should reclaim bytes each round");
    Expect(after.objects_reclaimed > before.objects_reclaimed,
           "gc stress profile should reclaim object records each round");
    Expect(after.bytes_live > 0,
           "gc stress profile should preserve reachable live bytes");

    min_live_bytes = std::min(min_live_bytes, after.bytes_live);
    max_live_bytes = std::max(max_live_bytes, after.bytes_live);
  }

  Expect(max_live_bytes <= min_live_bytes * 6 + 8192,
         "gc stress profile live-byte drift should stay within bounded threshold");

  std::cout << "[robustness][deterministic] suite=gc_stress rounds=" << kRounds
            << " min_live_bytes=" << min_live_bytes
            << " max_live_bytes=" << max_live_bytes << "\n";
}

void RunParserFuzzNondeterministicCase() {
  const std::vector<CorpusEntry> corpus = LoadCorpus();
  if (corpus.empty()) {
    return;
  }

  const int rounds = ResolveNondeterministicRounds();
  const std::uint32_t seed = ResolveNondeterministicSeed();

  std::mt19937 rng(seed);
  std::uniform_int_distribution<std::size_t> corpus_dist(0, corpus.size() - 1);

  std::size_t compile_errors = 0;
  std::size_t runtime_errors = 0;
  std::size_t ok_runs = 0;
  for (int round = 0; round < rounds; ++round) {
    const CorpusEntry& entry = corpus[corpus_dist(rng)];
    const std::filesystem::path seed_path = std::filesystem::path(RepoRoot()) / entry.path;
    const std::string source = ReadAll(seed_path);

    ms::Vm vm;
    vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
    std::ostringstream output;
    vm.set_output(output);

    std::string error;
    const ms::InterpretResult result =
        vm.execute_source_named(source, "tests/scripts/robustness/nondet_parser_fuzz.ms", &error);
    if (result == ms::InterpretResult::kCompileError) {
      ++compile_errors;
      Expect(!vm.last_diagnostics().empty(),
             "non-deterministic parser-fuzz compile error should emit diagnostics");
    } else if (result == ms::InterpretResult::kRuntimeError) {
      ++runtime_errors;
      Expect(!vm.last_diagnostics().empty(),
             "non-deterministic parser-fuzz runtime error should emit diagnostics");
    } else {
      ++ok_runs;
    }
  }

  std::cout << "[robustness][non_deterministic] suite=parser_fuzz_seed_sampling seed=" << seed
            << " rounds=" << rounds << " ok=" << ok_runs
            << " compile_errors=" << compile_errors
            << " runtime_errors=" << runtime_errors
            << " replay=\"ctest --test-dir build --output-on-failure -C Debug -R maple_tests_robustness_nondeterministic\""
            << "\n";
}

}  // namespace

int RunRobustnessDeterministicTests() {
  const std::optional<std::string> only_case = ReadEnvVar("MAPLE_ROBUSTNESS_CASE");
  if (only_case.has_value() && *only_case == "parser") {
    RunParserFuzzDeterministicCase();
    return 0;
  }
  if (only_case.has_value() && *only_case == "bytecode") {
    RunBytecodeInvariantDeterministicCase();
    return 0;
  }
  if (only_case.has_value() && *only_case == "gc") {
    RunGcStressDeterministicCase();
    return 0;
  }

  RunParserFuzzDeterministicCase();
  RunBytecodeInvariantDeterministicCase();
  RunGcStressDeterministicCase();
  return 0;
}

int RunRobustnessNondeterministicTests() {
  RunParserFuzzNondeterministicCase();
  return 0;
}

