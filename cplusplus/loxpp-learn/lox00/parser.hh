// Copyright (c) 2022 ASMlover. All rights reserved.
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

#include <vector>
#include "common.hh"
#include "token.hh"
#include "expr.hh"

namespace loxpp::parser {

// expression -> equality ;
// equality   -> comparison ( ( "!=" | "==" ) comparison )* ;
// comparison -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
// term       -> factor ( ( "-" | "+" ) factor )* ;
// factor     -> unary ( ( "/" | "*" ) unary )* ;
// unary      -> ( "!" | "-" ) unary | primary ;
// primary    -> NUMBER | STRING | "true" | "false" | "nil" | "(" expression ")" ;

class Parser final : private UnCopyable {
  std::vector<Token> tokens_;
  sz_t current_{};

  inline expr::ExprPtr expression() noexcept {
    // expression -> equality ;

    return equality();
  }

  inline expr::ExprPtr equality() noexcept {
    // equality -> comparison ( ( "!=" | "==" ) comparison )* ;

    expr::ExprPtr expr = comparison();
    return nullptr;
  }

  inline expr::ExprPtr comparison() noexcept {
    return nullptr;
  }
public:
  Parser(const std::vector<Token>& tokens) noexcept
    : tokens_{tokens} {
  }
};

}
