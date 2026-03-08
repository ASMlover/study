#pragma once

#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace ms {

struct SourceLocation {
  std::string file;
  std::size_t line = 1;
  std::size_t column = 1;
};

class SourceFile {
 public:
  static std::expected<SourceFile, std::string> LoadFromPath(
      const std::string& path);

  SourceFile(std::string path, std::string text);

  const std::string& Path() const;
  const std::string& Text() const;

  SourceLocation Locate(std::size_t offset) const;

 private:
  std::string path_;
  std::string text_;
  std::vector<std::size_t> line_starts_;
};

}  // namespace ms

