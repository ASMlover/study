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
TOKEN(ERROR, "error")
TOKEN(EOF, "eof")
TOKEN(IDENTIFIER, "identifier")
TOKEN(STRINGLITERAL, "string_literal")
TOKEN(NUMERICCONST, "numeric_const")

// punctuators
PUNCTUATOR(LPAREN, "(")
PUNCTUATOR(RPAREN, ")")
PUNCTUATOR(LBRACE, "{")
PUNCTUATOR(RBRACE, "}")
PUNCTUATOR(COMMA, ",")
PUNCTUATOR(DOT, ".")
PUNCTUATOR(SEMI, ";")
PUNCTUATOR(BANG, "!")
PUNCTUATOR(BANGEQUAL, "!=")
PUNCTUATOR(EQUAL, "=")
PUNCTUATOR(EQUALEQUAL, "==")
PUNCTUATOR(GREATER, ">")
PUNCTUATOR(GREATEREQUAL, ">=")
PUNCTUATOR(LESS, "<")
PUNCTUATOR(LESSEQUAL, "<=")
PUNCTUATOR(PLUS, "+")
PUNCTUATOR(MINUS, "-")
PUNCTUATOR(STAR, "*")
PUNCTUATOR(SLASH, "/")

// keywords
KEYWORD(AND, "and")
KEYWORD(CLASS, "class")
KEYWORD(ELSE, "else")
KEYWORD(FALSE, "false")
KEYWORD(FOR, "for")
KEYWORD(FUN, "fun")
KEYWORD(IF, "if")
KEYWORD(NIL, "nil")
KEYWORD(OR, "or")
// KEYWORD(PRINT, "print")
KEYWORD(RETURN, "return")
KEYWORD(SUPER, "super")
KEYWORD(THIS, "this")
KEYWORD(TRUE, "true")
KEYWORD(VAR, "var")
KEYWORD(WHILE, "while")

#undef KEYWORD
#undef PUNCTUATOR
#undef TOKEN
#undef TOKDEF
