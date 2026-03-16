#include "test_common.hh"

#include <string>

#include "cli/app.hh"

int RunCliTests() {
  const std::string ok_path = RepoRoot() + "/tests/scripts/cli_ok.ms";
  const char* argv_ok[] = {"maple_cli", ok_path.c_str()};
  int rc = ms::run_cli(2, const_cast<char**>(argv_ok));
  Expect(rc == 0, "cli should run script successfully");

  const std::string missing_path = RepoRoot() + "/tests/scripts/no_file.ms";
  const char* argv_missing[] = {"maple_cli", missing_path.c_str()};
  rc = ms::run_cli(2, const_cast<char**>(argv_missing));
  Expect(rc != 0, "cli should fail for missing file");
  return 0;
}