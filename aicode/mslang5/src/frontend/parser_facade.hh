#pragma once

#include <string>
#include <vector>

#include "frontend/token.hh"

namespace ms {

struct ParserBoundary {
  std::vector<Token> tokens;
};

ParserBoundary build_parser_boundary(const std::string& source);

}  // namespace ms
