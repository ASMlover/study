#pragma once

#include <filesystem>
#include <iostream>
#include <string>

inline int g_failures = 0;

inline void Expect(bool condition, const std::string& message) {
  if (!condition) {
    ++g_failures;
    std::cerr << "EXPECT FAILED: " << message << "\n";
  }
}

inline std::string RepoRoot() {
  const std::filesystem::path p(__FILE__);
  return p.parent_path().parent_path().parent_path().string();
}
