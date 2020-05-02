// Copyright (c) 2020 ASMlover. All rights reserved.
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
# define TOKDEF(K, S)
#endif

#ifndef TOKEN
# define TOKEN(K, S) TOKDEF(TK_##K, S)
#endif

#ifndef PUNCTUATOR
# define PUNCTUATOR(K, S) TOKEN(K, S)
#endif

#ifndef KEYWORD
# define KEYWORD(K, S) TOKDEF(KW_##K, S)
#endif

PUNCTUATOR(LPAREN, "(")
PUNCTUATOR(RPAREN, ")")
PUNCTUATOR(LBRACE, "{")
PUNCTUATOR(RBRACE, "}")
PUNCTUATOR(COMMA, ",")
PUNCTUATOR(MINUS, "-")
PUNCTUATOR(PLUS, "+")
PUNCTUATOR(SEMI, ";")
PUNCTUATOR(SLASH, "/")
PUNCTUATOR(STAR, "*")
PUNCTUATOR(EQ, "=")

TOKEN(IDENTIFIER, "IDENTIFIER")
TOKEN(NUMERIC, "NUMERIC")
TOKEN(STRING, "STRING")

KEYWORD(FALSE, "false")
KEYWORD(FN, "fn")
KEYWORD(NIL, "nil")
KEYWORD(TRUE, "true")
KEYWORD(VAR, "var")

TOKEN(EOF, "EOF")
TOKEN(ERR, "ERR")

#undef KEYWORD
#undef PUNCTUATOR
#undef TOKEN
#undef TOKDEF
