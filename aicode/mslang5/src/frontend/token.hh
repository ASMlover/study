#pragma once

#include <cstddef>
#include <string>

namespace ms {

enum class TokenType {
  kLeftParen,
  kRightParen,
  kLeftBrace,
  kRightBrace,
  kComma,
  kLess,
  kPlus,
  kMinus,
  kStar,
  kSlash,
  kSemicolon,
  kDot,
  kEqual,
  kIdentifier,
  kString,
  kNumber,
  kPrint,
  kVar,
  kImport,
  kFrom,
  kAs,
  kFun,
  kReturn,
  kClass,
  kThis,
  kSuper,
  kTrue,
  kFalse,
  kNil,
  kEof,
  kError,
};

struct Token {
  TokenType type = TokenType::kError;
  std::string lexeme;
  std::size_t line = 1;
};

}  // namespace ms
