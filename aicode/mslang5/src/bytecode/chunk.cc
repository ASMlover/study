#include "bytecode/chunk.hh"

namespace ms {

std::size_t Chunk::add_constant(Constant value) {
  constants_.push_back(std::move(value));
  return constants_.size() - 1;
}

void Chunk::write(const std::uint8_t byte, const std::size_t line) {
  code_.push_back(byte);
  lines_.push_back(line);
}

}  // namespace ms
