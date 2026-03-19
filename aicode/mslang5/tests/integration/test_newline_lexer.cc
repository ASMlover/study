#include "test_common.hh"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "frontend/lexer.hh"

namespace {

std::string ReadAll(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

struct NewlineCase {
  const char* relative_path;
  int expected_newline_tokens;
  bool expect_brace_boundaries;
  bool expect_trailing_newline_before_eof;
};

}  // namespace

int RunNewlineLexerIntegrationTests() {
  const std::vector<NewlineCase> cases = {
      {"tests/scripts/migration/newline/newline_brace_boundaries.ms", 4, true, true},
      {"tests/scripts/migration/newline/newline_comment_blank_lines.ms", 3, false, false},
  };

  for (const auto& test_case : cases) {
    const std::filesystem::path path = std::filesystem::path(RepoRoot()) / test_case.relative_path;
    const std::string source = ReadAll(path);
    ms::Lexer lexer(source, true);
    const std::vector<ms::Token> tokens = lexer.scan_all_tokens();

    int newline_count = 0;
    int left_brace_index = -1;
    int right_brace_index = -1;
    int eof_index = -1;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
      if (tokens[i].type == ms::TokenType::kNewline) {
        ++newline_count;
      }
      if (tokens[i].type == ms::TokenType::kLeftBrace) {
        left_brace_index = static_cast<int>(i);
      }
      if (tokens[i].type == ms::TokenType::kRightBrace) {
        right_brace_index = static_cast<int>(i);
      }
      if (tokens[i].type == ms::TokenType::kEof) {
        eof_index = static_cast<int>(i);
      }
    }

    const std::string label = path.filename().string();
    Expect(newline_count == test_case.expected_newline_tokens,
           label + " should preserve newline token count");

    if (test_case.expect_brace_boundaries) {
      Expect(left_brace_index > 0 && tokens[left_brace_index - 1].type == ms::TokenType::kNewline,
             label + " should keep newline before '{'");
      Expect(right_brace_index > 0 && tokens[right_brace_index - 1].type == ms::TokenType::kNewline,
             label + " should keep newline before '}'");
    }

    Expect(eof_index > 0, label + " should include eof token");
    if (eof_index > 0) {
      const bool has_newline_before_eof = tokens[eof_index - 1].type == ms::TokenType::kNewline;
      Expect(has_newline_before_eof == test_case.expect_trailing_newline_before_eof,
             label + " eof newline-boundary expectation mismatch");
    }
  }
  return 0;
}