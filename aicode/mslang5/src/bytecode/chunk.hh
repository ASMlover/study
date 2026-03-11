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
  inline void write_op(OpCode op, std::size_t line) {
    write(static_cast<std::uint8_t>(op), line);
  }

  inline const std::vector<std::uint8_t>& code() const noexcept { return code_; }
  inline std::vector<std::uint8_t>& mutable_code() noexcept { return code_; }
  inline const std::vector<std::size_t>& lines() const noexcept { return lines_; }
  inline const std::vector<Constant>& constants() const noexcept { return constants_; }

 private:
  std::vector<std::uint8_t> code_;
  std::vector<std::size_t> lines_;
  std::vector<Constant> constants_;
};

}  // namespace ms
