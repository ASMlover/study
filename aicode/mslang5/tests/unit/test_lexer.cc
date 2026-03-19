#include "test_common.hh"

#include <vector>

#include "frontend/lexer.hh"

int RunLexerTests() {
  ms::Lexer lexer("var x = 1; // c\nprint x;\nimport a.b;\nfrom a.b import c as d;\nif (x >= 1 and x <= 3 or x != 2) { while (x < 4) { x = x + 1; } }\nfor (var i = 0; i < 1; i = i + 1) {}");
  const std::vector<ms::Token> tokens = lexer.scan_all_tokens();
  Expect(!tokens.empty(), "lexer should emit tokens");
  Expect(tokens[0].type == ms::TokenType::kVar, "first token should be 'var'");

  bool has_import = false;
  bool has_from = false;
  bool has_if = false;
  bool has_while = false;
  bool has_for = false;
  bool has_and = false;
  bool has_or = false;
  bool has_ge = false;
  bool has_le = false;
  bool has_ne = false;
  bool has_newline = false;
  for (const auto& token : tokens) {
    if (token.type == ms::TokenType::kImport) {
      has_import = true;
    }
    if (token.type == ms::TokenType::kFrom) {
      has_from = true;
    }
    if (token.type == ms::TokenType::kIf) {
      has_if = true;
    }
    if (token.type == ms::TokenType::kWhile) {
      has_while = true;
    }
    if (token.type == ms::TokenType::kFor) {
      has_for = true;
    }
    if (token.type == ms::TokenType::kAnd) {
      has_and = true;
    }
    if (token.type == ms::TokenType::kOr) {
      has_or = true;
    }
    if (token.type == ms::TokenType::kGreaterEqual) {
      has_ge = true;
    }
    if (token.type == ms::TokenType::kLessEqual) {
      has_le = true;
    }
    if (token.type == ms::TokenType::kBangEqual) {
      has_ne = true;
    }
    if (token.type == ms::TokenType::kNewline) {
      has_newline = true;
    }
  }
  Expect(has_import, "lexer should recognize import keyword");
  Expect(has_from, "lexer should recognize from keyword");
  Expect(has_if, "lexer should recognize if keyword");
  Expect(has_while, "lexer should recognize while keyword");
  Expect(has_for, "lexer should recognize for keyword");
  Expect(has_and, "lexer should recognize and keyword");
  Expect(has_or, "lexer should recognize or keyword");
  Expect(has_ge, "lexer should recognize '>=' operator");
  Expect(has_le, "lexer should recognize '<=' operator");
  Expect(has_ne, "lexer should recognize '!=' operator");
  Expect(!has_newline, "lexer should not emit newline tokens by default");

  ms::Lexer newline_lexer("var x = 1;\n{\nprint x;\n}\n", true);
  const std::vector<ms::Token> newline_tokens = newline_lexer.scan_all_tokens();

  int newline_count = 0;
  int left_brace_index = -1;
  int right_brace_index = -1;
  int eof_index = -1;
  for (std::size_t i = 0; i < newline_tokens.size(); ++i) {
    if (newline_tokens[i].type == ms::TokenType::kNewline) {
      ++newline_count;
    }
    if (newline_tokens[i].type == ms::TokenType::kLeftBrace) {
      left_brace_index = static_cast<int>(i);
    }
    if (newline_tokens[i].type == ms::TokenType::kRightBrace) {
      right_brace_index = static_cast<int>(i);
    }
    if (newline_tokens[i].type == ms::TokenType::kEof) {
      eof_index = static_cast<int>(i);
    }
  }

  Expect(newline_count == 4, "newline lexer should emit one token per line break");
  Expect(left_brace_index > 0 && newline_tokens[left_brace_index - 1].type == ms::TokenType::kNewline,
         "left brace should follow a newline boundary");
  Expect(right_brace_index > 0 && newline_tokens[right_brace_index - 1].type == ms::TokenType::kNewline,
         "right brace should follow a newline boundary");
  Expect(eof_index > 0 && newline_tokens[eof_index - 1].type == ms::TokenType::kNewline,
         "eof should keep trailing newline boundary");
  return 0;
}