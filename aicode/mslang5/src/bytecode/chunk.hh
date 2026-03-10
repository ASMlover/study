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
  std::size_t add_constant(Constant value);
  void write(std::uint8_t byte, std::size_t line);
  void write_op(OpCode op, std::size_t line);

  const std::vector<std::uint8_t>& code() const;
  const std::vector<std::size_t>& lines() const;
  const std::vector<Constant>& constants() const;

 private:
  std::vector<std::uint8_t> code_;
  std::vector<std::size_t> lines_;
  std::vector<Constant> constants_;
};

}  // namespace ms
