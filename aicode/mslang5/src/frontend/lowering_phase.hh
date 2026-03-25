#pragma once

#include <string>
#include <vector>

#include "bytecode/chunk.hh"
#include "frontend/resolver_pass.hh"
#include "frontend/token.hh"

namespace ms {

struct LoweringInput {
  std::vector<Token> tokens;
  ResolverMetadata metadata;
};

struct LoweringResult {
  Chunk chunk;
  std::vector<std::string> errors;
};

LoweringResult run_lowering_phase(LoweringInput input);

}  // namespace ms
