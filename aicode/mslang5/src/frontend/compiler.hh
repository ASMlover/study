#pragma once

#include <string>
#include <vector>

#include "bytecode/chunk.hh"

namespace ms {

struct CompileResult {
  Chunk chunk;
  std::vector<std::string> errors;
};

CompileResult compile_to_chunk(const std::string& source);

}  // namespace ms

