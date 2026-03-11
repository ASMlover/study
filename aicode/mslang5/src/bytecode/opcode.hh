#pragma once

#include <cstdint>

namespace ms {

enum class OpCode : std::uint8_t {
  kConstant,
  kEqual,
  kGreater,
  kLess,
  kAdd,
  kSubtract,
  kMultiply,
  kDivide,
  kNot,
  kNegate,
  kPrint,
  kPop,
  kGetLocal,
  kSetLocal,
  kDefineGlobal,
  kGetGlobal,
  kSetGlobal,
  kJump,
  kJumpIfFalse,
  kLoop,
  kImportModule,
  kImportSymbol,
  kReturn,
};

}  // namespace ms
