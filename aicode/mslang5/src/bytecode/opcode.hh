#pragma once

#include <cstdint>

namespace ms {

enum class OpCode : std::uint8_t {
  kConstant,
  kAdd,
  kSubtract,
  kMultiply,
  kDivide,
  kNegate,
  kPrint,
  kPop,
  kDefineGlobal,
  kGetGlobal,
  kSetGlobal,
  kImportModule,
  kImportSymbol,
  kReturn,
};

}  // namespace ms

