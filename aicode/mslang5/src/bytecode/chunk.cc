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

void Chunk::write_op(const OpCode op, const std::size_t line) {
  write(static_cast<std::uint8_t>(op), line);
}

const std::vector<std::uint8_t>& Chunk::code() const noexcept { return code_; }

const std::vector<std::size_t>& Chunk::lines() const noexcept { return lines_; }

const std::vector<Constant>& Chunk::constants() const noexcept { return constants_; }

}  // namespace ms
