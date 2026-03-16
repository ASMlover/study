#include "test_common.hh"

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>

#include "runtime/vm.hh"

namespace {

std::size_t CountOccurrences(const std::string& text, const std::string& needle) {
  std::size_t count = 0;
  std::size_t pos = 0;
  while (true) {
    pos = text.find(needle, pos);
    if (pos == std::string::npos) {
      return count;
    }
    ++count;
    pos += needle.size();
  }
}

}  // namespace

int RunGcIntegrationTests() {
  ms::Vm vm;
  vm.modules().add_search_path(RepoRoot() + "/tests/scripts/module");
  std::ostringstream out;
  vm.set_output(out);
  vm.gc().set_threshold(128);

  std::string error;
  ms::InterpretResult r = vm.execute_source("import side;\n", &error);
  Expect(r == ms::InterpretResult::kOk, "module import before gc stress should execute");

  const std::string churn_script =
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

  constexpr int kRounds = 4;
  std::size_t min_live_bytes = std::numeric_limits<std::size_t>::max();
  std::size_t max_live_bytes = 0;

  for (int round = 0; round < kRounds; ++round) {
    const ms::GcStats before = vm.gc().stats();

    error.clear();
    r = vm.execute_source(churn_script, &error);
    Expect(r == ms::InterpretResult::kOk, "gc churn script should execute");
    Expect(vm.last_source_execution_route() == ms::SourceExecutionRoute::kVmPipeline,
           "gc churn script should run on VM pipeline");

    const ms::GcStats after = vm.gc().stats();
    Expect(after.collections > before.collections,
           "gc churn should trigger at least one collection per stress round");
    Expect(after.bytes_reclaimed > before.bytes_reclaimed,
           "gc churn should reclaim unreachable bytes per stress round");
    Expect(after.objects_reclaimed > before.objects_reclaimed,
           "gc churn should reclaim object records per stress round");
    Expect(after.bytes_live > 0, "gc should retain reachable objects as live bytes");

    min_live_bytes = std::min(min_live_bytes, after.bytes_live);
    max_live_bytes = std::max(max_live_bytes, after.bytes_live);
  }

  Expect(max_live_bytes <= min_live_bytes * 4 + 4096,
         "gc live-byte metrics should remain stable across repeated rounds");

  error.clear();
  r = vm.execute_source("import side;\n", &error);
  Expect(r == ms::InterpretResult::kOk, "module import after gc stress should execute");

  const std::string output = out.str();
  Expect(CountOccurrences(output, "init") == 1,
         "module cache should survive gc stress without re-initialization");
  Expect(CountOccurrences(output, "999") == kRounds,
         "repeated stress rounds should preserve reachable values each time");
  return 0;
}