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
// "AS IS" `AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
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
#include <string_view>
#include <unordered_map>
#include "token_type.h"

static std::unordered_map<std::string_view, TokenType> _keywords = {
  {"auto", TOKEN_AUTO},
  {"break", TOKEN_BREAK},
  {"case", TOKEN_CASE},
  {"char", TOKEN_CHAR},
  {"const", TOKEN_CONST},
  {"continue", TOKEN_CONTINUE},
  {"default", TOKEN_DEFAULT},
  {"do", TOKEN_DO},
  {"double", TOKEN_DOUBLE},
  {"else", TOKEN_ELSE},
  {"enum", TOKEN_ENUM},
  {"extern", TOKEN_EXTERN},
  {"float", TOKEN_FLOAT},
  {"for", TOKEN_FOR},
  {"goto", TOKEN_GOTO},
  {"if", TOKEN_IF},
  {"inline", TOKEN_INLINE},
  {"int", TOKEN_INT},
  {"long", TOKEN_LONG},
  {"register", TOKEN_REGISTER},
  {"restrict", TOKEN_RESTRICT},
  {"return", TOKEN_RETURN},
  {"short", TOKEN_SHORT},
  {"signed", TOKEN_SIGNED},
  {"sizeof", TOKEN_SIZEOF},
  {"static", TOKEN_STATIC},
  {"struct", TOKEN_STRUCT},
  {"switch", TOKEN_SWITCH},
  {"typedef", TOKEN_TYPEDEF},
  {"union", TOKEN_UNION},
  {"unsigned", TOKEN_UNSIGNED},
  {"void", TOKEN_VOID},
  {"volatile", TOKEN_VOLATILE},
  {"while", TOKEN_WHILE},
  {"_Alignas", TOKEN_ALIGNAS},
  {"_Alignof", TOKEN_ALIGNOF},
  {"_Atomic", TOKEN_ATOMIC},
  {"_Bool", TOKEN_BOOL},
  {"_Complex", TOKEN_COMPLEX},
  {"_Generic", TOKEN_GENERIC},
  {"_Imaginary", TOKEN_IMAGINARY},
  {"_Noreturn", TOKEN_NORETURN},
  {"_Static_assert", TOKEN_STATIC_ASSERT},
  {"_Thread_local", TOKEN_THREAD_LOCAL}
};

static std::unordered_map<std::string_view, TokenType> _macroKeywords = {
  {"#if", TOKEN_MACRO_IF},
  {"#elif", TOKEN_MACRO_ELIF},
  {"#else", TOKEN_MACRO_ELSE},
  {"#endif", TOKEN_MACRO_ENDIF},
  {"#defined", TOKEN_MACRO_DEFINED},
  {"#ifdef", TOKEN_MACRO_IFDEF},
  {"#define", TOKEN_MACRO_DEFINE},
  {"#undef", TOKEN_MACRO_UNDEF},
  {"#include", TOKEN_MACRO_INCLUDE},
  {"#line", TOKEN_MACRO_LINE},
  {"#error", TOKEN_MACRO_ERROR},
  {"#pragma", TOKEN_MACRO_PRAGMA}
};

const char* token_type_as_string(TokenType type) {
  switch (type) {
  case TOKEN_LPAREN: return "LPAREN `(`";
  case TOKEN_RPAREN: return "RPAREN `)`";
  case TOKEN_LBRACKET: return "LBRACKET `[`";
  case TOKEN_RBRACKET: return "RBRACKET `]`";
  case TOKEN_LBRACE: return "LBRACE `{`";
  case TOKEN_RBRACE: return "RBRACE `}`";
  case TOKEN_COMMA: return "COMMA `,`";
  case TOKEN_DOT: return "DOT `.`";
  case TOKEN_VARARGS: return "VARARGS `...`";
  case TOKEN_SEMICOLON: return "SEMICOLON `;`";
  case TOKEN_QUESTION: return "QUESTION `?`";
  case TOKEN_COLON: return "COLON `:`";
  case TOKEN_PLUS: return "PLUS `+`";
  case TOKEN_PLUS_EQUAL: return "PLUS_EQUAL `+=`";
  case TOKEN_PLUS_PLUS: return "PLUS_PLUS `++`";
  case TOKEN_MINUS: return "MINUS `-`";
  case TOKEN_MINUS_EQUAL: return "MINUS_EQUAL `-=`";
  case TOKEN_MINUS_MINUS: return "MINUS_MINUS `--`";
  case TOKEN_MINUS_GREATER: return "MINUS_GREATER `->`";
  case TOKEN_STAR: return "STAR `*`";
  case TOKEN_STAR_EQUAL: return "STAR_EQUAL `*=`";
  case TOKEN_SLASH: return "SLASH `/`";
  case TOKEN_SLASH_EQUAL: return "SLASH_EQUAL `/=`";
  case TOKEN_COMPLEMENT: return "COMPLEMENT `~`";
  case TOKEN_MODULO: return "MODULO `%`";
  case TOKEN_MODULO_EQUAL: return "MODULO_EQUAL `%=`";
  case TOKEN_AND: return "AND `&`";
  case TOKEN_AND_EQUAL: return "AND_EQUAL `&=`";
  case TOKEN_LOGIC_AND: return "LOGIC_AND `&&`";
  case TOKEN_OR: return "OR `|`";
  case TOKEN_OR_EQUAL: return "OR_EQUAL `|=`";
  case TOKEN_LOGIC_OR: return "LOGIC_OR `||`";
  case TOKEN_XOR: return "XOR `^`";
  case TOKEN_XOR_EQUAL: return "XOR_EQUAL `^=`";
  case TOKEN_LOGIC_NOT: return "LOGIC_NOT `!`";
  case TOKEN_NOT_EQUAL: return "NOT_EQUAL `!=`";
  case TOKEN_EQUAL: return "EQUAL `=`";
  case TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL `==`";
  case TOKEN_GREATER: return "GREATER `>`";
  case TOKEN_RSHIFT: return "RSHIFT `>>`";
  case TOKEN_RSHIFT_EQUAL: return "RSHIFT_EQUAL `>>=`";
  case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL `>=`";
  case TOKEN_LESS: return "LESS `<`";
  case TOKEN_LSHIFT: return "LSHIFT `<<`";
  case TOKEN_LSHIFT_EQUAL: return "LSHIFT_EQUAL `<<=`";
  case TOKEN_LESS_EQUAL: return "LESS_EQUAL `<=`";
  case TOKEN_IDENTIFILER: return "IDENTIFIER `identifier`";
  case TOKEN_INT_CONST: return "INT_CONST `integer-constant`";
  case TOKEN_UINT_CONST: return "UINT_CONST `unsigned-integer-constant`";
  case TOKEN_LONG_CONST: return "LONG_CONST `long-integer-constant`";
  case TOKEN_ULONG_CONST: return "ULONG_CONST `unsigned-long-integer-constant`";
  case TOKEN_LONGLONG_CONST: return "LONGLONG_CONST `longlong-integer-constant`";
  case TOKEN_ULONGLONG_CONST: return "ULONGLONG_CONST `unsigned-longlong-integer-constant`";
  case TOKEN_FLOAT_CONST: return "FLOAT_CONST `floating-constant`";
  case TOKEN_DOUBLE_CONST: return "DOUBLE_CONST `double-floating-constant`";
  case TOKEN_LONGDOUBLE_CONST: return "LONGDOUBLE_CONST `long-double-floating-constant`";
  case TOKEN_CHAR_CONST: return "CHAR_CONST `'c-char'`";
  case TOKEN_WCHAR_CONST: return "WCHAR_CONST `L'c-char'`";
  case TOKEN_STR_CONST: return "STR_CONST `\"s-char-sequence\"`";
  case TOKEN_WSTR_CONST: return "WSTR_CONST `L\"s-char-sequence\"`";
  case TOKEN_AUTO: return "AUTO `auto`";
  case TOKEN_BREAK: return "BREAK `break`";
  case TOKEN_CASE: return "CASE `case`";
  case TOKEN_CHAR: return "CHAR `char`";
  case TOKEN_CONST: return "CONST `const`";
  case TOKEN_CONTINUE: return "CONTINUE `continue`";
  case TOKEN_DEFAULT: return "DEFAULT `default`";
  case TOKEN_DO: return "DO `do`";
  case TOKEN_DOUBLE: return "DOUBLE `double`";
  case TOKEN_ELSE: return "ELSE `else`";
  case TOKEN_ENUM: return "ENUM `enum`";
  case TOKEN_EXTERN: return "EXTERN `extern`";
  case TOKEN_FLOAT: return "FLOAT `float`";
  case TOKEN_FOR: return "FOR `for`";
  case TOKEN_GOTO: return "GOTO `goto`";
  case TOKEN_IF: return "IF `if`";
  case TOKEN_INLINE: return "INLINE `inline`";
  case TOKEN_INT: return "INT `int`";
  case TOKEN_LONG: return "LONG `long`";
  case TOKEN_REGISTER: return "REGISTER `register`";
  case TOKEN_RESTRICT: return "RESTRICT `restrict`";
  case TOKEN_RETURN: return "RETURN `return`";
  case TOKEN_SHORT: return "SHORT `short`";
  case TOKEN_SIGNED: return "SIGNED `signed`";
  case TOKEN_SIZEOF: return "SIZEOF `sizeof`";
  case TOKEN_STATIC: return "STATIC `static`";
  case TOKEN_STRUCT: return "STRUCT `struct`";
  case TOKEN_SWITCH: return "SWITCH `switch`";
  case TOKEN_TYPEDEF: return "TYPEDEF `typedef`";
  case TOKEN_UNION: return "UNION `union`";
  case TOKEN_UNSIGNED: return "UNSIGNED `unsigned`";
  case TOKEN_VOID: return "VOID `void`";
  case TOKEN_VOLATILE: return "VOLATILE `volatile`";
  case TOKEN_WHILE: return "WHILE `while`";
  case TOKEN_ALIGNAS: return "_ALIGNAS `_Alignas`";
  case TOKEN_ALIGNOF: return "_ALIGNOF `_Alignof`";
  case TOKEN_ATOMIC: return "_ATOMIC `_Atomic`";
  case TOKEN_BOOL: return "_BOOL `_Bool`";
  case TOKEN_COMPLEX: return "_COMPLEX `_Complex`";
  case TOKEN_GENERIC: return "_GENERIC `_Generic`";
  case TOKEN_IMAGINARY: return "_IMAGINARY `_Imaginary`";
  case TOKEN_NORETURN: return "NORETURN `_Noreturn`";
  case TOKEN_STATIC_ASSERT: return "_STATIC_ASSERT `_Static_assert`";
  case TOKEN_THREAD_LOCAL: return "_THREAD_LOCAL `_Thread_local`";
  case TOKEN_MACRO_JOINT: return "MACRO_JOINT `##`";
  case TOKEN_MACRO_IF: return "MACRO_IF `#if`";
  case TOKEN_MACRO_ELIF: return "MACRO_ELIF `#elif`";
  case TOKEN_MACRO_ELSE: return "MACRO_ELSE `#else`";
  case TOKEN_MACRO_ENDIF: return "MACRO_ENDIF `#endif`";
  case TOKEN_MACRO_DEFINED: return "MACRO_DEFINED `#defined`";
  case TOKEN_MACRO_IFDEF: return "MACRO_IFDEF `#ifdef`";
  case TOKEN_MACRO_DEFINE: return "MACRO_DEFINE `#define`";
  case TOKEN_MACRO_UNDEF: return "MACRO_UNDEF `#undef`";
  case TOKEN_MACRO_INCLUDE: return "MACRO_INCLUDE `#include`";
  case TOKEN_MACRO_LINE: return "MACRO_LINE `#line`";
  case TOKEN_MACRO_ERROR: return "MACRO_ERROR `#error`";
  case TOKEN_MACRO_PRAGMA: return "MACRO_PRAGMA `#pragma`";
  case TOKEN_EOF: return "EOF";
  }
  return "";
}

bool is_keyword(const char* s) {
  return _keywords.find(s) == _keywords.end();
}

bool is_macro_keyword(const char* s) {
  return _macroKeywords.find(s) == _macroKeywords.end();
}

TokenType get_keyword_type(const char* s) {
  auto typeit = _keywords.find(s);
  return typeit->second;
}

TokenType get_macro_keyword_type(const char* s) {
  auto typeit = _macroKeywords.find(s);
  return typeit->second;
}
