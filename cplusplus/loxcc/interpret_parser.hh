// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include "lexer.hh"
#include "interpret_ast.hh"

namespace loxcc::interpret {

class ErrorReport;

// recursive descent parsing
class Parser final : private UnCopyable {
  ErrorReport& err_report_;
  Lexer& lex_;
  Token prev_;
  Token curr_;

  static constexpr sz_t kMaxArguments = 8;

  inline bool is_end(void) const { return curr_.kind() == TokenKind::TK_EOF; }

  Token advance(void);
  bool check(TokenKind kind) const;
  bool match(const std::initializer_list<TokenKind>& kinds);
  Token consume(TokenKind kind, const str_t& message);
  void synchronize(void);

  StmtPtr declaration(void);
  StmtPtr class_decl(void);
  StmtPtr fun_decl(const str_t& kind);
  StmtPtr var_decl(void);
  StmtPtr statement(void);
  StmtPtr expr_stmt(void);
  StmtPtr for_stmt(void);
  StmtPtr if_stmt(void);
  StmtPtr print_stmt(void);
  StmtPtr return_stmt(void);
  StmtPtr while_stmt(void);
  std::vector<StmtPtr> block_stmt(void);

  ExprPtr expression(void);
  ExprPtr assignment(void);
  ExprPtr logical_or(void);
  ExprPtr logical_and(void);
  ExprPtr equality(void);
  ExprPtr comparison(void);
  ExprPtr addition(void);
  ExprPtr multiplication(void);
  ExprPtr unary(void);
  ExprPtr call(void);
  ExprPtr primary(void);
public:
  Parser(ErrorReport& err_report, Lexer& lex) noexcept
    : err_report_(err_report), lex_(lex) {
  }

  std::vector<StmtPtr> parse(void);
};

}
