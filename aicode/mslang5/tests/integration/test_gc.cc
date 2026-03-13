#include "test_common.hh"

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

  error.clear();
  r = vm.execute_source(churn_script, &error);
  Expect(r == ms::InterpretResult::kOk, "gc churn script should execute");
  Expect(vm.last_source_execution_route() == ms::SourceExecutionRoute::kVmPipeline,
         "gc churn script should run on VM pipeline");

  const ms::GcStats stats = vm.gc().stats();
  Expect(stats.collections > 0, "gc churn should trigger collections");
  Expect(stats.bytes_reclaimed > 0, "gc churn should reclaim unreachable bytes");
  Expect(stats.objects_reclaimed > 0, "gc churn should reclaim unreachable object records");
  Expect(stats.bytes_live > 0, "gc should retain reachable objects as live bytes");

  error.clear();
  r = vm.execute_source("import side;\n", &error);
  Expect(r == ms::InterpretResult::kOk, "module import after gc stress should execute");

  const std::string output = out.str();
  Expect(CountOccurrences(output, "init") == 1,
         "module cache should survive gc stress without re-initialization");
  Expect(output.find("999") != std::string::npos,
         "reachable object graph should remain semantically valid after gc");
  return 0;
}