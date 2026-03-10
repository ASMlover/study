#pragma once

#include <string>

#include "bytecode/chunk.hh"

namespace ms {

std::string disassemble_chunk(const Chunk& chunk, const std::string& name);

}  // namespace ms

