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

int RunClosureIntegrationTests() {
  {
    ms::Vm vm;
    std::ostringstream out;
    vm.SetOutput(out);
    std::string error;
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_capture.ms");
    const ms::InterpretResult rc = vm.ExecuteSource(src, &error);
    Expect(rc == ms::InterpretResult::kOk, "closure_capture should execute");
    Expect(out.str() == "11\n12\n", "closure_capture output should be 11,12");
  }

  {
    ms::Vm vm;
    std::ostringstream out;
    vm.SetOutput(out);
    std::string error;
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_lexical.ms");
    const ms::InterpretResult rc = vm.ExecuteSource(src, &error);
    Expect(rc == ms::InterpretResult::kOk, "closure_lexical should execute");
    Expect(out.str() == "local\nglobal\n", "closure_lexical output should preserve lexical scope");
  }

  {
    ms::Vm vm;
    std::ostringstream out;
    vm.SetOutput(out);
    std::string error;
    const std::string src = ReadAll(RepoRoot() + "/tests/scripts/language/closure_arity_error.ms");
    const ms::InterpretResult rc = vm.ExecuteSource(src, &error);
    Expect(rc == ms::InterpretResult::kRuntimeError, "closure_arity_error should fail");
    Expect(error.find("expected 2 arguments but got 1") != std::string::npos,
           "arity mismatch should report actual and expected arguments");
  }
  return 0;
}
