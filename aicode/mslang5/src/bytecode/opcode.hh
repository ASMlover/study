#pragma once

#include <cstdint>

namespace ms {

enum class OpCode : std::uint8_t {
  kConstant,
  kClosure,
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
  kGetUpvalue,
  kSetUpvalue,
  kCall,
  kInvoke,
  kSuperInvoke,
  kCloseUpvalue,
  kClass,
  kInherit,
  kMethod,
  kGetProperty,
  kSetProperty,
  kGetSuper,
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

