#include "support/source.hh"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
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

Diagnostic MakeDiagnostic(std::string phase, std::string code, std::string message,
                          DiagnosticSpan span, std::vector<std::string> notes) {
  return Diagnostic{
      std::move(phase),
      std::move(code),
      std::move(message),
      std::move(span),
      std::move(notes),
  };
}

namespace {

std::string Trim(const std::string& value) {
  std::size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
    ++start;
  }
  std::size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
    --end;
  }
  return value.substr(start, end - start);
}

}  // namespace

Diagnostic ParseDiagnosticText(const std::string& text, const std::string& default_phase,
                               const std::string& default_code,
                               const std::string& default_file) {
  std::string primary = text;
  const std::size_t newline = primary.find('\n');
  if (newline != std::string::npos) {
    primary = primary.substr(0, newline);
  }
  primary = Trim(primary);

  std::smatch match;
  const std::regex with_line_and_code(
      R"(^\s*\[line\s+([0-9]+)\]\s+([a-zA-Z]+)\s+error\s+\((MS[0-9]{4})\):\s*(.+)\s*$)");
  if (std::regex_match(primary, match, with_line_and_code)) {
    DiagnosticSpan span{default_file, static_cast<std::size_t>(std::stoul(match[1].str())),
                        std::nullopt, std::nullopt};
    return MakeDiagnostic(match[2].str(), match[3].str(), Trim(match[4].str()), span);
  }

  const std::regex with_line_without_code(
      R"(^\s*\[line\s+([0-9]+)\]\s+([a-zA-Z]+)\s+error:\s*(.+)\s*$)");
  if (std::regex_match(primary, match, with_line_without_code)) {
    DiagnosticSpan span{default_file, static_cast<std::size_t>(std::stoul(match[1].str())),
                        std::nullopt, std::nullopt};
    return MakeDiagnostic(match[2].str(), default_code, Trim(match[3].str()), span);
  }

  const std::regex without_line_with_code(
      R"(^\s*([a-zA-Z]+)\s+error\s+\((MS[0-9]{4})\):\s*(.+)\s*$)");
  if (std::regex_match(primary, match, without_line_with_code)) {
    DiagnosticSpan span{default_file, 1, std::nullopt, std::nullopt};
    return MakeDiagnostic(match[1].str(), match[2].str(), Trim(match[3].str()), span);
  }

  const std::regex module_prefix(R"(^\s*module\s+error\s+\((MS[0-9]{4})\):\s*(.+)\s*$)");
  if (std::regex_match(primary, match, module_prefix)) {
    DiagnosticSpan span{default_file, 1, std::nullopt, std::nullopt};
    return MakeDiagnostic("module", match[1].str(), Trim(match[2].str()), span);
  }

  DiagnosticSpan span{default_file, 1, std::nullopt, std::nullopt};
  return MakeDiagnostic(default_phase, default_code, primary, span);
}

std::string RenderDiagnostic(const Diagnostic& diagnostic) {
  std::ostringstream out;
  out << "[" << diagnostic.phase << " " << diagnostic.code << "] " << diagnostic.message << "\n";
  out << diagnostic.span.file << ":" << diagnostic.span.line;
  if (diagnostic.span.column.has_value()) {
    out << ":" << *diagnostic.span.column;
  }
  for (const auto& note : diagnostic.notes) {
    out << "\nnote: " << note;
  }
  return out.str();
}

std::string RenderDiagnostics(const std::vector<Diagnostic>& diagnostics) {
  std::ostringstream out;
  for (std::size_t i = 0; i < diagnostics.size(); ++i) {
    if (i != 0) {
      out << "\n";
    }
    out << RenderDiagnostic(diagnostics[i]);
  }
  return out.str();
}

}  // namespace ms
