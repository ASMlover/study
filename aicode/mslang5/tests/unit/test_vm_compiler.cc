#include "test_common.hh"

#include <sstream>
#include <string>

#include "runtime/vm.hh"

int RunVmCompilerTests() {
  ms::Vm vm;
  std::ostringstream out;
  vm.SetOutput(out);

  std::string error;
  const std::string script =
      "var x = 2 + 3 * 4;\n"
      "print x;\n"
      "x = x + 1;\n"
      "print x;\n";

  const ms::InterpretResult r = vm.ExecuteSource(script, &error);
  Expect(r == ms::InterpretResult::kOk, "vm should execute arithmetic script");
  Expect(out.str().find("14") != std::string::npos, "first print should be 14");
  Expect(out.str().find("15") != std::string::npos, "second print should be 15");

  vm.Gc().SetThreshold(1);
  vm.DefineGlobal("gc_probe", ms::Value(1.0));
  Expect(vm.Gc().Stats().collections > 0, "gc collection should trigger");
  return 0;
}

