#include <iostream>
#include <string>

#include "test_common.hh"

int RunLoggerTests();
int RunSourceTests();
int RunChunkDisasmTests();
int RunLexerTests();
int RunVmCompilerTests();
int RunModuleTests();
int RunCliTests();
int RunClosureIntegrationTests();
int RunClassIntegrationTests();
int RunResolverIntegrationTests();
int RunMigrationDebtTests();
int RunConformanceIntegrationTests();
int RunDiagnosticsGoldenTests();

namespace {

void RunUnitSuite() {
  RunLoggerTests();
  RunSourceTests();
  RunChunkDisasmTests();
  RunLexerTests();
  RunVmCompilerTests();
  RunModuleTests();
}

void RunIntegrationSuite() {
  RunCliTests();
  RunClosureIntegrationTests();
  RunClassIntegrationTests();
  RunResolverIntegrationTests();
}

void RunMigrationDebtSuite() { RunMigrationDebtTests(); }

void RunConformanceSuite() { RunConformanceIntegrationTests(); }

void RunDiagnosticsSuite() { RunDiagnosticsGoldenTests(); }

}  // namespace

int main(int argc, char** argv) {
  std::string suite = "all";
  if (argc > 1) {
    suite = argv[1];
  }

  if (suite == "--suite=all" || suite == "all") {
    RunUnitSuite();
    RunIntegrationSuite();
    RunMigrationDebtSuite();
    RunConformanceSuite();
    RunDiagnosticsSuite();
  } else if (suite == "--suite=unit" || suite == "unit") {
    RunUnitSuite();
  } else if (suite == "--suite=integration" || suite == "integration") {
    RunIntegrationSuite();
  } else if (suite == "--suite=migration_debt" || suite == "migration_debt") {
    RunMigrationDebtSuite();
  } else if (suite == "--suite=conformance" || suite == "conformance") {
    RunConformanceSuite();
  } else if (suite == "--suite=diagnostics" || suite == "diagnostics") {
    RunDiagnosticsSuite();
  } else {
    std::cerr << "Unknown suite argument: " << suite << "\n";
    return 2;
  }

  if (g_failures != 0) {
    std::cerr << "Total failures: " << g_failures << "\n";
    return 1;
  }
  std::cout << "All tests passed\n";
  return 0;
}
