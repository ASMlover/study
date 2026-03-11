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
