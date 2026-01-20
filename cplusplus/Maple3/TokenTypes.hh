// Copyright (c) 2026 ASMlover. All rights reserved.
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

#if !defined(TOKDEF)
# define TOKDEF(k, s)
#endif

#if !defined(TOKEN)
# define TOKEN(k, s)              TOKDEF(TK_##k, s)
#endif

#if !defined(PUNCTUATOR)
# define PUNCTUATOR(k, s)         TOKEN(k, s)
#endif

#if !defined(KEYWORD)
# define KEYWORD(k, s)            TOKDEF(KW_##k, s)
#endif

// single-character tokens
PUNCTUATOR(LPAREN,                "(")
PUNCTUATOR(RPAREN,                ")")
PUNCTUATOR(LBRACE,                "{")
PUNCTUATOR(RBRACE,                "}")
PUNCTUATOR(COMMA,                 ",")
PUNCTUATOR(DOT,                   ".")
PUNCTUATOR(MINUS,                 "-")
PUNCTUATOR(PLUS,                  "+")
PUNCTUATOR(SEMICOLON,             ";")
PUNCTUATOR(SLASH,                 "/")
PUNCTUATOR(STAR,                  "*")

// one or two character tokens
PUNCTUATOR(BANG,                  "!")
PUNCTUATOR(BANG_EQUAL,            "!=")
PUNCTUATOR(EQUAL,                 "=")
PUNCTUATOR(EQUAL_EQUAL,           "==")
PUNCTUATOR(GREATER,               ">")
PUNCTUATOR(GREATER_EQUAL,         ">=")
PUNCTUATOR(LESS,                  "<")
PUNCTUATOR(LESS_EQUAL,            "<=")

// literals
TOKEN(IDENTIFIER,                 "Identifier")
TOKEN(STRING,                     "String")
TOKEN(NUMBER,                     "Numeric")

// keywords
KEYWORD(AND,                      "and")
KEYWORD(AS,                       "as")
KEYWORD(CLASS,                    "class")
KEYWORD(ELSE,                     "else")
KEYWORD(FALSE,                    "false")
KEYWORD(FOR,                      "for")
KEYWORD(FUN,                      "fun")
KEYWORD(IF,                       "if")
KEYWORD(IMPORT,                   "import")
KEYWORD(NIL,                      "nil")
KEYWORD(OR,                       "or")
KEYWORD(PRINT,                    "print")
KEYWORD(RETURN,                   "return")
KEYWORD(SUPER,                    "super")
KEYWORD(THIS,                     "this")
KEYWORD(TRUE,                     "true")
KEYWORD(VAR,                      "var")
KEYWORD(WHILE,                    "while")

TOKEN(EOF,                        "Eof")
TOKEN(ERR,                        "Error")

#undef KEYWORD
#undef PUNCTUATOR
#undef TOKEN
#undef TOKDEF
