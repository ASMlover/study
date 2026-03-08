#include "test_common.hh"

#include <filesystem>
#include <fstream>

#include "support/source.hh"

int RunSourceTests() {
  const std::string path =
      (std::filesystem::temp_directory_path() / "maple_tmp_source_test.ms").string();
  {
    std::ofstream out(path, std::ios::binary);
    out << "a\nbc\n";
  }

  auto source = ms::SourceFile::LoadFromPath(path);
  Expect(source.has_value(), "source file should load");
  if (source) {
    const ms::SourceLocation loc = source->Locate(3);
    Expect(loc.line == 2, "line mapping should work");
    Expect(loc.column == 2, "column mapping should work");
  }

  auto missing = ms::SourceFile::LoadFromPath(RepoRoot() + "/tests/scripts/not_found.ms");
  Expect(!missing.has_value(), "missing file should fail");
  std::filesystem::remove(path);
  return 0;
}
