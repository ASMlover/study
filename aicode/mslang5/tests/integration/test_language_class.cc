#include "test_common.hh"

#include <fstream>
#include <sstream>
#include <string>

#include "runtime/vm.hh"

namespace {

std::string ReadAll(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

}  // namespace

int RunClassIntegrationTests() {
  {
    ms::Vm vm;
    std::ostringstream out;
    vm.SetOutput(out);
    std::string error;
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_fields.ms");
    const ms::InterpretResult rc = vm.ExecuteSource(src, &error);
    Expect(rc == ms::InterpretResult::kOk, "class_fields should execute");
    Expect(out.str() == "7\n9\n", "class_fields output should be 7,9");
  }

  {
    ms::Vm vm;
    std::ostringstream out;
    vm.SetOutput(out);
    std::string error;
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_inherit.ms");
    const ms::InterpretResult rc = vm.ExecuteSource(src, &error);
    Expect(rc == ms::InterpretResult::kOk, "class_inherit should execute");
    Expect(out.str() == "AB\n", "class_inherit output should be AB");
  }

  {
    ms::Vm vm;
    std::ostringstream out;
    vm.SetOutput(out);
    std::string error;
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/class_super_error.ms");
    const ms::InterpretResult rc = vm.ExecuteSource(src, &error);
    Expect(rc == ms::InterpretResult::kRuntimeError, "class_super_error should fail");
    Expect(error.find("undefined property: nope") != std::string::npos,
           "missing super method should report undefined property");
  }
  return 0;
}
