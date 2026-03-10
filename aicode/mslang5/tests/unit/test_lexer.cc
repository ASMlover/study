#include "test_common.hh"

#include <vector>

#include "frontend/lexer.hh"

int RunLexerTests() {
  ms::Lexer lexer("var x = 1; // c\nprint x;\nimport a.b;\nfrom a.b import c as d;");
  const std::vector<ms::Token> tokens = lexer.scan_all_tokens();
  Expect(!tokens.empty(), "lexer should emit tokens");
  Expect(tokens[0].type == ms::TokenType::kVar, "first token should be 'var'");

  bool has_import = false;
  bool has_from = false;
  for (const auto& token : tokens) {
    if (token.type == ms::TokenType::kImport) {
      has_import = true;
    }
    if (token.type == ms::TokenType::kFrom) {
      has_from = true;
    }
  }
  Expect(has_import, "lexer should recognize import keyword");
  Expect(has_from, "lexer should recognize from keyword");
  return 0;
}

