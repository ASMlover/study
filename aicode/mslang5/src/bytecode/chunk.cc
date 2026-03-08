#include "bytecode/chunk.hh"

namespace ms {

std::size_t Chunk::AddConstant(Constant value) {
  constants_.push_back(std::move(value));
  return constants_.size() - 1;
}

void Chunk::Write(const std::uint8_t byte, const std::size_t line) {
  code_.push_back(byte);
  lines_.push_back(line);
}

void Chunk::WriteOp(const OpCode op, const std::size_t line) {
  Write(static_cast<std::uint8_t>(op), line);
}

const std::vector<std::uint8_t>& Chunk::Code() const { return code_; }

const std::vector<std::size_t>& Chunk::Lines() const { return lines_; }

const std::vector<Constant>& Chunk::Constants() const { return constants_; }

}  // namespace ms

