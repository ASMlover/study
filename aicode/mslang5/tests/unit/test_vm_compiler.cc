#include "test_common.hh"

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>

#include "runtime/vm.hh"

int RunVmCompilerTests() {
  ms::Vm vm;
  std::ostringstream out;
  vm.set_output(out);

  std::string error;
  const std::string script =
      "var x = 2 + 3 * 4;\n"
      "print x;\n"
      "x = x + 1;\n"
      "print x;\n"
      "{\n"
      "  var x = 3;\n"
      "  if (x < 5 and x != 4) {\n"
      "    print x;\n"
      "  } else {\n"
      "    print 999;\n"
      "  }\n"
      "}\n"
      "var sum = 0;\n"
      "for (var i = 0; i < 5; i = i + 1) {\n"
      "  if (i == 2 or i == 4) {\n"
      "    sum = sum + 10;\n"
      "  } else {\n"
      "    sum = sum + i;\n"
      "  }\n"
      "}\n"
      "while (sum < 30) {\n"
      "  sum = sum + 1;\n"
      "}\n"
      "print sum;\n";

  const ms::InterpretResult r = vm.execute_source(script, &error);
  Expect(r == ms::InterpretResult::kOk, "vm should execute arithmetic script");
  Expect(vm.last_source_execution_route() == ms::SourceExecutionRoute::kVmPipeline,
         "arithmetic script should execute on VM pipeline");
  Expect(out.str().find("14") != std::string::npos, "first print should be 14");
  Expect(out.str().find("15") != std::string::npos, "second print should be 15");
  Expect(out.str().find("3") != std::string::npos, "if/and/!= branch should print 3");
  Expect(out.str().find("30") != std::string::npos, "for/while control flow should print 30");

  out.str("");
  out.clear();
  error.clear();
  const std::string closure_script =
      "fun makeCounter(start) {\n"
      "  var value = start;\n"
      "  fun inc() {\n"
      "    value = value + 1;\n"
      "    return value;\n"
      "  }\n"
      "  return inc;\n"
      "}\n"
      "var c = makeCounter(10);\n"
      "print c();\n"
      "print c();\n";
  const ms::InterpretResult closure_result = vm.execute_source(closure_script, &error);
  Expect(closure_result == ms::InterpretResult::kOk, "closure script should execute");
  Expect(vm.last_source_execution_route() == ms::SourceExecutionRoute::kVmPipeline,
         "closure script should execute on VM pipeline");
  Expect(out.str() == "11\n12\n", "closure script output should be stable");

  error.clear();
  const ms::InterpretResult compile_error = vm.execute_source("var x = ;\n", &error);
  Expect(compile_error == ms::InterpretResult::kCompileError,
         "invalid declaration should map to compile error");
  Expect(!error.empty(), "compile error should expose parser diagnostics");

  error.clear();
  const ms::InterpretResult runtime_error = vm.execute_source("print missing_name;\n", &error);
  Expect(runtime_error == ms::InterpretResult::kRuntimeError,
         "undefined variable should map to runtime error");
  Expect(error.find("MS4001") != std::string::npos,
         "undefined variable should expose MS4001 code");
  Expect(error.find("undefined variable") != std::string::npos,
         "runtime error should retain undefined variable detail");

  out.str("");
  out.clear();
  error.clear();
  const ms::InterpretResult native_ok = vm.execute_source(
      "print native_require_number(41);\n"
      "print native_clock() > 0;\n",
      &error);
  Expect(native_ok == ms::InterpretResult::kOk, "native callable script should execute");
  Expect(out.str().find("41") != std::string::npos,
         "native callable should return valid value");
  Expect(out.str().find("true") != std::string::npos,
         "native clock should be callable and produce comparable number");

  error.clear();
  const ms::InterpretResult native_arity_error = vm.execute_source("native_clock(1);\n", &error);
  Expect(native_arity_error == ms::InterpretResult::kRuntimeError,
         "native arity mismatch should raise runtime error");
  Expect(error.find("MS4002") != std::string::npos,
         "native arity mismatch should expose MS4002 code");

  error.clear();
  const ms::InterpretResult native_type_error =
      vm.execute_source("native_require_number(\"oops\");\n", &error);
  Expect(native_type_error == ms::InterpretResult::kRuntimeError,
         "native type mismatch should raise runtime error");
  Expect(error.find("MS4003") != std::string::npos,
         "native type mismatch should expose MS4003 code");
  Expect(error.find("number") != std::string::npos,
         "native type mismatch should keep argument-kind detail");

  const std::string gc_round_script =
      "class Node { init(v, next) { this.v = v; this.next = next; } }\n"
      "fun build(limit) {\n"
      "  var head = nil;\n"
      "  var i = 0;\n"
      "  while (i < limit) {\n"
      "    head = Node(i, head);\n"
      "    i = i + 1;\n"
      "  }\n"
      "  return head;\n"
      "}\n"
      "var keep = build(120);\n"
      "var churn = 0;\n"
      "while (churn < 35) { build(40); churn = churn + 1; }\n"
      "print keep.v;\n";

  constexpr int kGcRounds = 4;
  std::size_t min_collections = std::numeric_limits<std::size_t>::max();
  std::size_t max_collections = 0;
  std::size_t min_reclaimed = std::numeric_limits<std::size_t>::max();
  std::size_t max_reclaimed = 0;

  for (int round = 0; round < kGcRounds; ++round) {
    ms::Vm gc_vm;
    std::ostringstream gc_out;
    gc_vm.set_output(gc_out);
    gc_vm.gc().set_threshold(128);

    error.clear();
    const ms::InterpretResult gc_round_result = gc_vm.execute_source(gc_round_script, &error);
    Expect(gc_round_result == ms::InterpretResult::kOk,
           "gc stress round script should execute successfully");

    const ms::GcStats stats = gc_vm.gc().stats();
    Expect(stats.collections > 0, "gc stress round should trigger collections");
    Expect(stats.bytes_reclaimed > 0, "gc stress round should reclaim bytes");
    Expect(stats.objects_reclaimed > 0, "gc stress round should reclaim objects");

    min_collections = std::min(min_collections, stats.collections);
    max_collections = std::max(max_collections, stats.collections);
    min_reclaimed = std::min(min_reclaimed, stats.bytes_reclaimed);
    max_reclaimed = std::max(max_reclaimed, stats.bytes_reclaimed);
  }

  Expect(max_collections <= min_collections * 2 + 2,
         "gc collection count should remain stable across repeated runs");
  Expect(max_reclaimed <= min_reclaimed * 4 + 4096,
         "gc reclaimed-byte metrics should remain stable across repeated runs");
  return 0;
}