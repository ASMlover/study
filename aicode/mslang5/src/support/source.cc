#include "support/source.hh"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace ms {

std::expected<SourceFile, std::string> SourceFile::LoadFromPath(
    const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open()) {
    return std::unexpected("failed to open source file: " + path);
  }
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return SourceFile(path, buffer.str());
}

SourceFile::SourceFile(std::string path, std::string text)
    : path_(std::move(path)), text_(std::move(text)) {
  line_starts_.push_back(0);
  for (std::size_t i = 0; i < text_.size(); ++i) {
    if (text_[i] == '\n') {
      line_starts_.push_back(i + 1);
    }
  }
}

const std::string& SourceFile::Path() const { return path_; }

const std::string& SourceFile::Text() const { return text_; }

SourceLocation SourceFile::Locate(const std::size_t offset) const {
  const std::size_t bounded = std::min(offset, text_.size());
  const auto it =
      std::upper_bound(line_starts_.begin(), line_starts_.end(), bounded);
  const std::size_t line_index =
      (it == line_starts_.begin()) ? 0 : (std::size_t)(it - line_starts_.begin() - 1);
  const std::size_t line_start = line_starts_[line_index];
  return SourceLocation{path_, line_index + 1, bounded - line_start + 1};
}

}  // namespace ms

