#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "bytecode/opcode.hh"

namespace ms {

using Constant = std::variant<std::monostate, bool, double, std::string>;

class Chunk {
 public:
  std::size_t AddConstant(Constant value);
  void Write(std::uint8_t byte, std::size_t line);
  void WriteOp(OpCode op, std::size_t line);

  const std::vector<std::uint8_t>& Code() const;
  const std::vector<std::size_t>& Lines() const;
  const std::vector<Constant>& Constants() const;

 private:
  std::vector<std::uint8_t> code_;
  std::vector<std::size_t> lines_;
  std::vector<Constant> constants_;
};

}  // namespace ms
