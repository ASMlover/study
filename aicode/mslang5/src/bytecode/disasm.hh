#pragma once

#include <string>

#include "bytecode/chunk.hh"

namespace ms {

std::string DisassembleChunk(const Chunk& chunk, const std::string& name);

}  // namespace ms

