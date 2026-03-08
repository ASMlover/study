#include <iostream>

#include "test_common.hh"

int RunLoggerTests();
int RunSourceTests();
int RunChunkDisasmTests();
int RunLexerTests();
int RunVmCompilerTests();
int RunModuleTests();
int RunCliTests();

int main() {
  RunLoggerTests();
  RunSourceTests();
  RunChunkDisasmTests();
  RunLexerTests();
  RunVmCompilerTests();
  RunModuleTests();
  RunCliTests();

  if (g_failures != 0) {
    std::cerr << "Total failures: " << g_failures << "\n";
    return 1;
  }
  std::cout << "All tests passed\n";
  return 0;
}

