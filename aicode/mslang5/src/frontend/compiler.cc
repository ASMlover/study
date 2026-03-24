#include "frontend/compiler.hh"

#include <utility>

#include "frontend/lowering_phase.hh"
#include "frontend/parser_facade.hh"
#include "frontend/resolver_pass.hh"

namespace ms {

CompileResult compile_to_chunk(const std::string& source) {
  ParserBoundary boundary = build_parser_boundary(source);
  ResolverPassResult resolver_result = run_resolver_pass(std::move(boundary.tokens));

  LoweringInput lowering_input;
  lowering_input.tokens = std::move(resolver_result.tokens);
  lowering_input.metadata = resolver_result.metadata;

  LoweringResult lowering_result = run_lowering_phase(std::move(lowering_input));

  CompileResult result;
  result.chunk = std::move(lowering_result.chunk);
  result.errors = std::move(lowering_result.errors);
  result.errors.insert(result.errors.end(), resolver_result.errors.begin(), resolver_result.errors.end());
  return result;
}

}  // namespace ms
