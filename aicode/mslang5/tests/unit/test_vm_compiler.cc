#include "test_common.hh"

#include <sstream>
#include <string>

#include "runtime/vm.hh"

int RunVmCompilerTests() {
  ms::Vm vm;
  std::ostringstream out;
  vm.set_output(out);
  Expect(vm.get_source_execution_mode() == ms::SourceExecutionMode::kVmPreferredWithLegacyFallback,
         "default execution mode should prefer VM pipeline");

  std::string error;
  const std::string script =
      "var x = 2 + 3 * 4;\n"
      "print x;\n"
      "x = x + 1;\n"
      "print x;\n";

  const ms::InterpretResult r = vm.execute_source(script, &error);
  Expect(r == ms::InterpretResult::kOk, "vm should execute arithmetic script");
  Expect(vm.last_source_execution_route() == ms::SourceExecutionRoute::kVmPipeline,
         "arithmetic script should execute on VM pipeline");
  Expect(out.str().find("14") != std::string::npos, "first print should be 14");
  Expect(out.str().find("15") != std::string::npos, "second print should be 15");

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

  vm.gc().set_threshold(1);
  vm.define_global("gc_probe", ms::Value(1.0));
  Expect(vm.gc().stats().collections > 0, "gc collection should trigger");
  return 0;
}
