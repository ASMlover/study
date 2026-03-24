#include "test_common.hh"

#include <sstream>
#include <string>
#include <vector>

#include "frontend/lexer.hh"
#include "frontend/parser.hh"
#include "frontend/parser_facade.hh"

namespace {

void AssertTokensEqual(const std::vector<ms::Token>& expected,
                       const std::vector<ms::Token>& actual) {
  Expect(expected.size() == actual.size(), "parser facade token count should remain stable");
  const std::size_t count = expected.size() < actual.size() ? expected.size() : actual.size();
  for (std::size_t i = 0; i < count; ++i) {
    std::ostringstream msg;
    msg << "token mismatch at index " << i;
    const bool same = expected[i].type == actual[i].type && expected[i].lexeme == actual[i].lexeme &&
                      expected[i].line == actual[i].line;
    Expect(same, msg.str());
  }
}

void ParseImportStatement(ms::Parser* parser) {
  const bool has_import = parser->match(ms::TokenType::kImport);
  Expect(has_import, "test setup should start with import statement");
  (void)parser->parse_dotted_name();
  (void)parser->consume_statement_end("expected statement end after import statement");
}

}  // namespace

int RunParserFacadeTests() {
  const std::string stable_source =
      "import pkg.math\n"
      "from pkg.math import abs as abs_alias\n"
      "var value = 41\n"
      "value = value + 1\n"
      "print value\n";

  ms::Lexer lexer(stable_source, true);
  const std::vector<ms::Token> direct_tokens = lexer.scan_all_tokens();

  const ms::ParserBoundary boundary = ms::build_parser_boundary(stable_source);
  AssertTokensEqual(direct_tokens, boundary.tokens);

  const std::string invalid_import_source = "import pkg.\n";
  ms::Lexer direct_invalid_lexer(invalid_import_source, true);
  ms::Parser direct_parser(direct_invalid_lexer.scan_all_tokens());

  const ms::ParserBoundary invalid_boundary = ms::build_parser_boundary(invalid_import_source);
  ms::Parser facade_parser(invalid_boundary.tokens);

  ParseImportStatement(&direct_parser);
  ParseImportStatement(&facade_parser);

  const std::vector<std::string>& direct_errors = direct_parser.errors();
  const std::vector<std::string>& facade_errors = facade_parser.errors();
  Expect(direct_errors == facade_errors,
         "parser facade should preserve parser error diagnostics for invalid import");
  Expect(!facade_errors.empty(), "invalid import should produce parser diagnostic");
  if (!facade_errors.empty()) {
    const bool has_detail = facade_errors.front().find("expected identifier after '.'") !=
                            std::string::npos;
    Expect(has_detail, "invalid import diagnostic should preserve detail");
  }

  return 0;
}
