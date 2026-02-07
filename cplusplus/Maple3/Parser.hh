// Copyright (c) 2026 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include <initializer_list>
#include "Ast.hh"
#include "Common.hh"
#include "Errors.hh"
#include "Token.hh"

namespace ms {

class Parser final : private UnCopyable {
  const std::vector<Token>& tokens_;
  sz_t current_{};
  str_t source_fname_;
  std::vector<str_t> errors_;

  bool is_at_end() const noexcept;
  const Token& peek() const noexcept;
  const Token& previous() const noexcept;
  const Token& advance() noexcept;
  bool check(TokenType type) const noexcept;
  bool match(std::initializer_list<TokenType> types) noexcept;

  ParseError parse_error(const Token& token, const str_t& message) noexcept;
  void synchronize() noexcept;
  const Token& consume(TokenType type, const str_t& message);

  ExprPtr expression();
  ExprPtr equality();
  ExprPtr comparison();
  ExprPtr term();
  ExprPtr factor();
  ExprPtr unary();
  ExprPtr primary();
public:
  Parser(const std::vector<Token>& tokens, const str_t& source_fname = "") noexcept;

  ExprPtr parse_expression() noexcept;
  ExprList parse_program() noexcept;

  inline bool has_error() const noexcept { return !errors_.empty(); }
  inline const std::vector<str_t>& errors() const noexcept { return errors_; }
};

}
