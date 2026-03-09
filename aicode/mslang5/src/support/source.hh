#pragma once

#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ms {

struct SourceLocation {
  std::string file;
  std::size_t line = 1;
  std::size_t column = 1;
};

struct DiagnosticSpan {
  std::string file;
  std::size_t line = 1;
  std::optional<std::size_t> column;
  std::optional<std::size_t> length;
};

struct Diagnostic {
  std::string phase;
  std::string code;
  std::string message;
  DiagnosticSpan span;
  std::vector<std::string> notes;
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

Diagnostic MakeDiagnostic(std::string phase, std::string code, std::string message,
                          DiagnosticSpan span,
                          std::vector<std::string> notes = {});
Diagnostic ParseDiagnosticText(const std::string& text, const std::string& default_phase,
                               const std::string& default_code,
                               const std::string& default_file);
std::string RenderDiagnostic(const Diagnostic& diagnostic);
std::string RenderDiagnostics(const std::vector<Diagnostic>& diagnostics);

}  // namespace ms
