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
#include "../common/common.hh"
#include "../lex/token_kinds.hh"
#include "ast.hh"

namespace sage {

// parogram       -> declaration* EOF ;
// declaration    -> class_decl | func_decl | let_decl | statement ;
// class_decl     -> "class" IDENTIFIER ( "(" IDENTIFIER ")" )? "{" NL func_decl* "}" NL ;
// func_decl      -> "fn" IDENTIFIER "(" parameters? ")" block_stmt ;
// parameters     -> IDENTIFIER ( "," IDENTIFIER )* ;
// let_decl       -> "let" IDENTIFIER ( "=" expression )? NL ;
// statement      -> if_stmt | while_stmt | for_stmt | return_stmt | break_stmt
//                 | print_stmt | block_stmt | expr_stmt ;
// if_stmt        -> "if" expression block_stmt ( "else" block_stmt )? ;
// while_stmt     -> "while" expression block_stmt ;
// for_stmt       -> "for" init_clause expression? ";" expression? block_stmt ;
// init_clause    -> loop_let | loop_expr | ";" ;
// loop_let       -> "let" IDENTIFIER ( "=" expression )? ";" ;
// loop_expr      -> expression ";" ;
// return_stmt    -> "return" expression? NL ;
// break_stmt     -> "break" NL ;
// print_stmt     -> "print" ( expression ( "," expression )* )? NL ;
// block_stmt     -> "{" NL declaration* "}" NL ;
// expr_stmt      -> expression NL ;

// expression     -> assignment ;
// assignment     -> ( call "." )? IDENTIFIER ( assign_oper ) assignment
//                 | logic_or ;
// assign_oper    -> "=" | "+=" | "-=" | "*=" | "/=" | "%=" ;
// logic_or       -> logic_and ( "or" logic_and )* ;
// logic_and      -> equality ( "and" equality )* ;
// equality       -> comparison ( ( "is" | "!=" | "==" ) comparison )* ;
// comparison     -> addition ( ( ">" | ">=" | "<" | "<=" ) addition )* ;
// addition       -> multiplication ( ( "+" | "-" ) multiplication )* ;
// multiplication -> unary ( ( "*" | "/" | "%" ) unary )* ;
// unary          -> ( "not" | "!" | "-" ) unary | call ;
// call           -> primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
// arguments      -> expression ( "," expression )* ;
// primary        -> INTEGER | DECIMAL | STRING
//                 | "true" | "false" | "nil" | "self"
//                 | "(" expression ")" | "super" "." IDENTIFIER ;

class Token;
class ErrorReport;

class Parser : private UnCopyable {
  ErrorReport& err_report_;
  std::vector<Token> tokens_;
  std::size_t curpos_{};

  static constexpr std::size_t kMaxArguments = 64;

  bool is_end(void) const;
  const Token& peek(void) const;
  const Token& prev(void) const;
  const Token& advance(void);
  bool check(TokenKind kind) const;
  bool match(const std::initializer_list<TokenKind>& kinds);
  const Token& consume(TokenKind kind, const std::string& message);
  const Token& consume(const std::initializer_list<TokenKind>& kinds,
      const std::string& message);

  void synchronize(void);
  bool ignore_newlines(void);

  StmtPtr declaration(void);
  StmtPtr class_decl(void);
  StmtPtr func_decl(const std::string& kind);
  StmtPtr let_decl(void);
  StmtPtr statement(void);
  StmtPtr if_stmt(void);
  StmtPtr while_stmt(void);
  StmtPtr for_stmt(void);
  StmtPtr return_stmt(void);
  StmtPtr break_stmt(void);
  StmtPtr print_stmt(void);
  std::vector<StmtPtr> block_stmt(void);
  StmtPtr expr_stmt(void);

  ExprPtr expression(void);
  ExprPtr assignment(void);
  ExprPtr logic_or(void);
  ExprPtr logic_and(void);
  ExprPtr equality(void);
  ExprPtr comparison(void);
  ExprPtr addition(void);
  ExprPtr multiplication(void);
  ExprPtr unary(void);
  ExprPtr call(void);
  ExprPtr primary(void);
public:
  Parser(ErrorReport& err_report, const std::vector<Token>& tokens);

  ExprPtr parse(void);
  std::vector<StmtPtr> parse_stmts(void);
};

}
