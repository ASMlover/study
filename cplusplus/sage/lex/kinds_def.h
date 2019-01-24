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

#ifndef TOKDEF
# define TOKDEF(k, s)
#endif

#ifndef TOKEN
# define TOKEN(k, s) TOKDEF(TK_##k, s)
#endif

#ifndef PUNCTUATOR
# define PUNCTUATOR(k, s) TOKEN(k, s)
#endif

#ifndef KEYWORD
# define KEYWORD(k, s) TOKDEF(KW_##k, s)
#endif

// language tokens
TOKEN(UNKNOWN, "unknown") // unknown token
TOKEN(ERROR, "error") // error token
TOKEN(EOF, "eof") // end of file
TOKEN(NL, "newline") // newline
TOKEN(COMMENT, "comment") // comment
TOKEN(IDENTIFIER, "identifier")
TOKEN(INTEGERCONST, "integer_constant")
TOKEN(DECIMALCONST, "decimal_constant")
TOKEN(STRINGLITERAL, "string_literal")

// punctuators
PUNCTUATOR(LSQUARE, "[")
PUNCTUATOR(RSQUARE, "]")
PUNCTUATOR(LPAREN, "(")
PUNCTUATOR(RPAREN, ")")
PUNCTUATOR(LBRACE, "{")
PUNCTUATOR(RBRACE, "}")
PUNCTUATOR(PERIOD, ".")
PUNCTUATOR(PERIODPERIOD, "..")
PUNCTUATOR(PERIODPERIODPERIOD, "...")
PUNCTUATOR(COMMA, ",")
PUNCTUATOR(COLON, ":")
PUNCTUATOR(SEMI, ";")
PUNCTUATOR(PLUS, "+")
PUNCTUATOR(PLUSEQUAL, "+=")
PUNCTUATOR(MINUS, "-")
PUNCTUATOR(MINUSEQUAL, "-=")
PUNCTUATOR(STAR, "*")
PUNCTUATOR(STAREQUAL, "*=")
PUNCTUATOR(SLASH, "/")
PUNCTUATOR(SLASHEQUAL, "/=")
PUNCTUATOR(PERCENT, "%")
PUNCTUATOR(PERCENTEQUAL, "%=")
PUNCTUATOR(LESS, "<")
PUNCTUATOR(LESSEQUAL, "<=")
PUNCTUATOR(GREATER, ">")
PUNCTUATOR(GREATEREQUAL, ">=")
PUNCTUATOR(EXCLAIM, "!")
PUNCTUATOR(EXCLAIMEQUAL, "!=")
PUNCTUATOR(EQUAL, "=")
PUNCTUATOR(EQUALEQUAL, "==")

// keywords
KEYWORD(NIL, "nil")
KEYWORD(TRUE, "true")
KEYWORD(FALSE, "false")
KEYWORD(AND, "and")
KEYWORD(OR, "or")
KEYWORD(NOT, "not")
KEYWORD(IS, "is")
KEYWORD(IN, "in")
KEYWORD(IF, "if")
KEYWORD(ELIF, "elif")
KEYWORD(ELSE, "else")
KEYWORD(WHILE, "while")
KEYWORD(FOR, "for")
KEYWORD(BREAK, "break")
KEYWORD(FN, "fn")
KEYWORD(RETURN, "return")
KEYWORD(CLASS, "class")
KEYWORD(SELF, "self")
KEYWORD(SUPER, "super")
KEYWORD(STATIC, "static")
KEYWORD(LET, "let")
KEYWORD(IMPORT, "import")
KEYWORD(PRINT, "print")

#undef KEYWORD
#undef PUNCTUATOR
#undef TOKEN
#undef TOKDEF
