#include "frontend/parser_facade.hh"

#include "frontend/lexer.hh"

namespace ms {

ParserBoundary build_parser_boundary(const std::string& source) {
  Lexer lexer(source, true);
  ParserBoundary boundary;
  boundary.tokens = lexer.scan_all_tokens();
  return boundary;
}

}  // namespace ms
