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

#ifndef PPKEYWORD
# define PPKEYWORD(K, S)
#endif

#ifndef TOKEN
# define TOKEN(K, S)
#endif

#ifndef PUNCTUATOR
# define PUNCTUATOR(K, S) TOKEN(K, S)
#endif

#ifndef KEYWORD
# define KEYWORD(K, S) TOKEN(KW_##K, S)
#endif

// preprocessor keywords
PPKEYWORD(IF, "#if")
PPKEYWORD(IFDEF, "#ifdef")
PPKEYWORD(IFNDEF, "#ifndef")
PPKEYWORD(ELIF, "#elif")
PPKEYWORD(ELSE, "#else")
PPKEYWORD(ENDIF, "#endif")
PPKEYWORD(DEFINED, "#defined")
PPKEYWORD(DEFINE, "#define")
PPKEYWORD(UNDEF, "#undef")
PPKEYWORD(INCLUDE, "#include")
PPKEYWORD(LINE, "#line")
PPKEYWORD(ERROR, "#error")
PPKEYWORD(PRAGMA, "#pragma")

// language keywords
TOKEN(UNKNOWN, "unknown") // not a token
TOKEN(ENDF, "eof") // end of file
TOKEN(COMMENT, "comment") // comment
TOKEN(IDENTIFILER, "identifiler")
TOKEN(INTCONST, "int_constant")
TOKEN(UINTCONST, "uint_constant")
TOKEN(LONGCONST, "long_constant")
TOKEN(ULONGCONST, "ulong_constant")
TOKEN(LLONGCONST, "llong_constant")
TOKEN(ULLONGCONST, "ullong_constant")
TOKEN(FLOATCONST, "float_constant")
TOKEN(DOUBLECONST, "double_constant")
TOKEN(LDOUBLECONST, "ldouble_constant")
TOKEN(CHARCONST, "char_constant") // 'a'
TOKEN(WCHARCONST, "wchar_constant") // L'a'
TOKEN(U8CHARCONST, "u8char_constant") // u8'a'
TOKEN(U16CHARCONST, "u16char_constant") // u'a'
TOKEN(U32CHARCONST, "u32char_constant") // U'a'
TOKEN(STRINGLITERAL, "string_literal") // "foo"
TOKEN(WSTRINGLITERAL, "wstring_literal") // L"foo"
TOKEN(ANGLESTRINGLITERAL, "anglestring_literal") // <foo>
TOKEN(U8STRINGLITERAL, "u8string_literal") // u8"foo"
TOKEN(U16STRINGLITERAL, "u16string_literal") // u"foo"
TOKEN(U32STRINGLITERAL, "u32string_literal") // U"foo"

// punctuators
PUNCTUATOR(LSQUARE, "[")
PUNCTUATOR(RSQUARE, "]")
PUNCTUATOR(LPAREN, "(")
PUNCTUATOR(RPAREN, ")")
PUNCTUATOR(LBRACE, "{")
PUNCTUATOR(RBRACE, "}")
PUNCTUATOR(PERIOD, ".")
PUNCTUATOR(ELLIPSIS, "...")
PUNCTUATOR(AMP, "&")
PUNCTUATOR(AMPAMP, "&&")
PUNCTUATOR(AMPEQUAL, "&=")
PUNCTUATOR(STAR, "*")
PUNCTUATOR(STAREQUAL, "*=")
PUNCTUATOR(PLUS, "+")
PUNCTUATOR(PLUSPLUS, "++")
PUNCTUATOR(PLUSEQUAL, "+=")
PUNCTUATOR(MINUS, "-")
PUNCTUATOR(MINUSMINUS, "--")
PUNCTUATOR(MINUSEQUAL, "-=")
PUNCTUATOR(ARROW, "->")
PUNCTUATOR(TILDE, "~")
PUNCTUATOR(EXCLAIM, "!")
PUNCTUATOR(EXCLAIMEQUAL, "!=")
PUNCTUATOR(SLASH, "/")
PUNCTUATOR(SLASHEQUAL, "/=")
PUNCTUATOR(PERCENT, "%")
PUNCTUATOR(PERCENTEQUAL, "%=")
PUNCTUATOR(LESS, "<")
PUNCTUATOR(LESSLESS, "<<")
PUNCTUATOR(LESSEQUAL, "<=")
PUNCTUATOR(LESSLESSEQUAL, "<<=")
PUNCTUATOR(GREATER, ">")
PUNCTUATOR(GREATERGREATER, ">>")
PUNCTUATOR(GREATEREQUAL, ">=")
PUNCTUATOR(GREATERGREATEREQUAL, ">>=")
PUNCTUATOR(CARET, "^")
PUNCTUATOR(CARETEQUAL, "^=")
PUNCTUATOR(PIPE, "|")
PUNCTUATOR(PIPEPIPE, "||")
PUNCTUATOR(PIPEEQUAL, "|=")
PUNCTUATOR(QUESTION, "?")
PUNCTUATOR(COLON, ":")
PUNCTUATOR(SEMI, ";")
PUNCTUATOR(EQUAL, "=")
PUNCTUATOR(EQUALEQUAL, "==")
PUNCTUATOR(COMMA, ",")
PUNCTUATOR(HASH, "#")
PUNCTUATOR(HASHHASH, "##")

// keywords
KEYWORD(AUTO, "auto")
KEYWORD(BREAK, "break")
KEYWORD(CASE, "case")
KEYWORD(CHAR, "char")
KEYWORD(CONST, "const")
KEYWORD(CONTINUE, "continue")
KEYWORD(DEFAULT, "default")
KEYWORD(DO, "do")
KEYWORD(DOUBLE, "double")
KEYWORD(ELSE, "else")
KEYWORD(ENUM, "enum")
KEYWORD(EXTERN, "extern")
KEYWORD(FLOAT, "float")
KEYWORD(FOR, "for")
KEYWORD(GOTO, "goto")
KEYWORD(IF, "if")
KEYWORD(INLINE, "inline")
KEYWORD(INT, "int")
KEYWORD(LONG, "long")
KEYWORD(REGISTER, "register")
KEYWORD(RESTRICT, "restrict")
KEYWORD(RETURN, "return")
KEYWORD(SHORT, "short")
KEYWORD(SIGNED, "signed")
KEYWORD(SIZEOF, "sizeof")
KEYWORD(STATIC, "static")
KEYWORD(STRUCT, "struct")
KEYWORD(SWITCH, "switch")
KEYWORD(TYPEDEF, "typedef")
KEYWORD(UNION, "union")
KEYWORD(UNSIGNED, "unsigned")
KEYWORD(VOID, "void")
KEYWORD(VOLATILE, "volatile")
KEYWORD(WHILE, "while")
KEYWORD(_ALIGNAS, "_Alignas")
KEYWORD(_ALIGNOF, "_Alignof")
KEYWORD(_ATOMIC, "_Atomic")
KEYWORD(_BOOL, "_Bool")
KEYWORD(_COMPLEX, "_Complex")
KEYWORD(_GENERIC, "_Generic")
KEYWORD(_IMAGINARY, "_Imaginary")
KEYWORD(_NORETURN, "_Noreturn")
KEYWORD(_STATIC_ASSERT, "_Static_assert")
KEYWORD(_THREAD_LOCAL, "_Thread_local")
KEYWORD(__FUNC__, "__func__")
KEYWORD(WCHART, "wchar_t")

#undef KEYWORD
#undef PUNCTUATOR
#undef TOKEN
#undef PPKEYWORD
