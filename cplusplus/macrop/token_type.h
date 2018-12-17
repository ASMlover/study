// Copyright (c) 2018 ASMlover. All rights reserved.
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

enum TokenType {
  TOKEN_LPAREN, // (
  TOKEN_RPAREN, // )
  TOKEN_LBRACKET, // [
  TOKEN_RBRACKET, // ]
  TOKEN_LBRACE, // {
  TOKEN_RBRACE, // }
  TOKEN_COMMA, // ,
  TOKEN_DOT, // .
  TOKEN_VARARGS, // ...
  TOKEN_SEMICOLON, // ;
  TOKEN_QUESTION, // ?
  TOKEN_COLON, // :
  TOKEN_PLUS, // +
  TOKEN_PLUS_EQUAL, // +=
  TOKEN_PLUS_PLUS, // ++
  TOKEN_MINUS, // -
  TOKEN_MINUS_EQUAL, // -=
  TOKEN_MINUS_MINUS, // --
  TOKEN_MINUS_GREATER, // ->
  TOKEN_STAR, // *
  TOKEN_STAR_EQUAL, // *=
  TOKEN_SLASH, // /
  TOKEN_SLASH_EQUAL, // /=
  TOKEN_COMPLEMENT, // ~
  TOKEN_MODULO, // %
  TOKEN_MODULO_EQUAL, // %=
  TOKEN_MACRO_JOINT, // ##
  TOKEN_AND, // &
  TOKEN_AND_EQUAL, // &=
  TOKEN_LOGIC_AND, // &&
  TOKEN_OR, // |
  TOKEN_OR_EQUAL, // |=
  TOKEN_LOGIC_OR, // ||
  TOKEN_XOR, // ^
  TOKEN_XOR_EQUAL, // ^=
  TOKEN_LOGIC_NOT, // !
  TOKEN_NOT_EQUAL, // !=
  TOKEN_EQUAL, // =
  TOKEN_EQUAL_EQUAL, // ==
  TOKEN_GREATER, // >
  TOKEN_RSHIFT, // >>
  TOKEN_RSHIFT_EQUAL, // >>=
  TOKEN_GREATER_EQUAL, // >=
  TOKEN_LESS, // <
  TOKEN_LSHIFT, // <<
  TOKEN_LSHIFT_EQUAL, // <<=
  TOKEN_LESS_EQUAL, // <=

  TOKEN_IDENTIFILER,
  TOKEN_INT_CONST,

  TOKEN_UINT_CONST,
  TOKEN_LONG_CONST,
  TOKEN_ULONG_CONST,
  TOKEN_LONGLONG_CONST,
  TOKEN_ULONGLONG_CONST,

  TOKEN_FLOAT_CONST,
  TOKEN_DOUBLE_CONST,
  TOKEN_LONGDOUBLE_CONST,
  TOKEN_CHAR_CONST, // 'c-char'
  TOKEN_WCHAR_CONST, // L'c-char'
  TOKEN_STR_CONST, // "s-char-sequence"
  TOKEN_WSTR_CONST, // L"s-char-sequence"

  TOKEN_AUTO, // auto
  TOKEN_BREAK, // break
  TOKEN_CASE, // case
  TOKEN_CHAR, // char
  TOKEN_CONST, // const
  TOKEN_CONTINUE, // continue
  TOKEN_DEFAULT, // default
  TOKEN_DO, // do
  TOKEN_DOUBLE, // double
  TOKEN_ELSE, // else
  TOKEN_ENUM, // enum
  TOKEN_EXTERN, // extern
  TOKEN_FLOAT, // float
  TOKEN_FOR, // for
  TOKEN_GOTO, // goto
  TOKEN_IF, // if
  TOKEN_INLINE, // inline
  TOKEN_INT, // int
  TOKEN_LONG, // long
  TOKEN_REGISTER, // register
  TOKEN_RESTRICT, // restrict
  TOKEN_RETURN, // return
  TOKEN_SHORT, // short
  TOKEN_SIGNED, // signed
  TOKEN_SIZEOF, // sizeof
  TOKEN_STATIC, // static
  TOKEN_STRUCT, // struct
  TOKEN_SWITCH, // switch
  TOKEN_TYPEDEF, // typedef
  TOKEN_UNION, // union
  TOKEN_UNSIGNED, // unsigned
  TOKEN_VOID, // void
  TOKEN_VOLATILE, // volatile
  TOKEN_WHILE, // while
  TOKEN_ALIGNAS, // _Alignas
  TOKEN_ALIGNOF, // _Alignof
  TOKEN_ATOMIC, // _Atomic
  TOKEN_BOOL, // _Bool
  TOKEN_COMPLEX, // _Complex
  TOKEN_GENERIC, // _Generic
  TOKEN_IMAGINARY, // _Imaginary
  TOKEN_NORETURN, // _Noreturn
  TOKEN_STATIC_ASSERT, // _Static_assert
  TOKEN_THREAD_LOCAL, // _Thread_local

  TOKEN_MACRO_IF, // #if
  TOKEN_MACRO_ELIF, // #elif
  TOKEN_MACRO_ELSE, // #else
  TOKEN_MACRO_ENDIF, // #endif
  TOKEN_MACRO_DEFINED, // #defined
  TOKEN_MACRO_IFDEF, // #ifdef
  TOKEN_MACRO_DEFINE, // #define
  TOKEN_MACRO_UNDEF, // #undef
  TOKEN_MACRO_INCLUDE, // #include
  TOKEN_MACRO_LINE, // #line
  TOKEN_MACRO_ERROR, // #error
  TOKEN_MACRO_PRAGMA, // #pragma

  TOKEN_EOF
};

const char* token_type_as_string(TokenType type);
bool is_keyword(const char* s);
bool is_macro_keyword(const char* s);
TokenType get_keyword_type(const char* s);
TokenType get_macro_keyword_type(const char* s);
