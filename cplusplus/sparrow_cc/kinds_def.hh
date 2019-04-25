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

TOKEN(UNKNOWN, "unknown")
// language data types
TOKEN(NUMERIC, "numeric")
TOKEN(STRING, "string")
TOKEN(IDENTIFIER, "indentifier")
TOKEN(INTERPOLATION, "interpolation")

// keywords
KEYWORD(VAR, "var")
KEYWORD(FUN, "fun")
KEYWORD(IF, "if")
KEYWORD(ELSE, "else")
KEYWORD(TRUE, "true")
KEYWORD(FALSE, "false")
KEYWORD(WHILE, "while")
KEYWORD(FOR, "for")
KEYWORD(BREAK, "break")
KEYWORD(CONTINUE, "continue")
KEYWORD(RETURN, "return")
KEYWORD(NIL, "nil")

KEYWORD(CLASS, "class")
KEYWORD(THIS, "this")
KEYWORD(STATIC, "static")
KEYWORD(IS, "is")
KEYWORD(SUPER, "super")
KEYWORD(IMPORT, "import")

// punctuators
PUNCTUATOR(COMMA, ",")
PUNCTUATOR(COLON, ":")
PUNCTUATOR(LPAREN, "(")
PUNCTUATOR(RPAREN, ")")
PUNCTUATOR(LBRACKET, "[")
PUNCTUATOR(RBRACKET, "]")
PUNCTUATOR(LBRACE, "{")
PUNCTUATOR(RBRACE, "}")
PUNCTUATOR(DOT, ".")
PUNCTUATOR(DOTDOT, "..")

// binary operations
PUNCTUATOR(PLUS, "+")
PUNCTUATOR(MINUS, "-")
PUNCTUATOR(STAR, "*")
PUNCTUATOR(SLASH, "/")
PUNCTUATOR(PERCENT, "%")

// assignment
PUNCTUATOR(EQUAL, "=")

// bit operations
PUNCTUATOR(BITAND, "&")
PUNCTUATOR(BITOR, "|")
PUNCTUATOR(BITNOT, "~")
PUNCTUATOR(RSHIFT, ">>")
PUNCTUATOR(LSHIFT, "<<")

// logical operations
PUNCTUATOR(LOGICAND, "&&")
PUNCTUATOR(LOGICOR, "||")
PUNCTUATOR(LOGICNOT, "!")

// compare operations
PUNCTUATOR(EQUALEQUAL, "==")
PUNCTUATOR(NOTEQUAL, "!=")
PUNCTUATOR(GREATER, ">")
PUNCTUATOR(GREATEREQUAL, ">=")
PUNCTUATOR(LESS, "<")
PUNCTUATOR(LESSEQUAL, "<=")

PUNCTUATOR(QUESTION, "?")

TOKEN(EOF, "eof")

#undef KEYWORD
#undef PUNCTUATOR
#undef TOKEN
#undef TOKDEF
