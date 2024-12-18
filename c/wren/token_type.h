/*
 * Copyright (c) 2024 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined(WREN_TOKENDEF)
#	define WREN_TOKENDEF(k, s)
#endif

#if !defined(WREN_TOKEN)
#	define WREN_TOKEN(k, s)                 WREN_TOKENDEF(TOKEN_##k, s)
#endif

#if !defined(WREN_PUNCTUATOR)
#	define WREN_PUNCTUATOR(k, s)            WREN_TOKEN(k, s)
#endif

#if !defined(WREN_KEYWORD)
#	define WREN_KEYWORD(k, s)               WREN_TOKENDEF(KEYWORD_##k, s)
#endif

WREN_PUNCTUATOR(LEFT_PAREN, "(")
WREN_PUNCTUATOR(RIGHT_PAREN, ")")
WREN_PUNCTUATOR(LEFT_BRACKET, "[")
WREN_PUNCTUATOR(RIGHT_BRACKET, "]")
WREN_PUNCTUATOR(LEFT_BRACE, "{")
WREN_PUNCTUATOR(RIGHT_BRACE, "}")
WREN_PUNCTUATOR(COLON, ":")
WREN_PUNCTUATOR(DOT, ".")
WREN_PUNCTUATOR(DOTDOT, "..")
WREN_PUNCTUATOR(DOTDOTDOT, "...")
WREN_PUNCTUATOR(COMMA, ",")
WREN_PUNCTUATOR(STAR, "*")
WREN_PUNCTUATOR(SLASH, "/")
WREN_PUNCTUATOR(PERCENT, "%")
WREN_PUNCTUATOR(PLUS, "+")
WREN_PUNCTUATOR(MINUS, "-")
WREN_PUNCTUATOR(LTLT, "<<")
WREN_PUNCTUATOR(GTGT, ">>")
WREN_PUNCTUATOR(PIPE, "|")
WREN_PUNCTUATOR(PIPEPIPE, "||")
WREN_PUNCTUATOR(CARET, "^")
WREN_PUNCTUATOR(AMP, "&")
WREN_PUNCTUATOR(AMPAMP, "&&")
WREN_PUNCTUATOR(BANG, "!")
WREN_PUNCTUATOR(TILDE, "~")
WREN_PUNCTUATOR(QUESTION, "?")
WREN_PUNCTUATOR(EQ, "=")
WREN_PUNCTUATOR(LT, "<")
WREN_PUNCTUATOR(GT, ">")
WREN_PUNCTUATOR(LTEQ, "<=")
WREN_PUNCTUATOR(GTEQ, ">=")
WREN_PUNCTUATOR(EQEQ, "==")
WREN_PUNCTUATOR(BANGEQ, "!=")

WREN_KEYWORD(BREAK, "break")
WREN_KEYWORD(CLASS, "class")
WREN_KEYWORD(CONSTRUCT, "construct")
WREN_KEYWORD(ELSE, "else")
WREN_KEYWORD(FALSE, "false")
WREN_KEYWORD(FOR, "for")
WREN_KEYWORD(FOREIGN, "foreign")
WREN_KEYWORD(IF, "if")
WREN_KEYWORD(IMPORT, "import")
WREN_KEYWORD(IN, "in")
WREN_KEYWORD(IS, "is")
WREN_KEYWORD(NULL, "null")
WREN_KEYWORD(RETURN, "return")
WREN_KEYWORD(STATIC, "static")
WREN_KEYWORD(SUPER, "super")
WREN_KEYWORD(THIS, "this")
WREN_KEYWORD(TRUE, "true")
WREN_KEYWORD(VAR, "var")
WREN_KEYWORD(WHILE, "while")

WREN_TOKEN(FIELD, "Field")
WREN_TOKEN(STATIC_FIELD, "StaticField")
WREN_TOKEN(NAME, "Name")
WREN_TOKEN(NUMBER, "Number")
WREN_TOKEN(STRING, "String")

WREN_TOKEN(INTERPOLATION, "Interpolation")
WREN_TOKEN(LINE, "Newline")

WREN_TOKEN(ERROR, "Error")
WREN_TOKEN(EOF, "Eof")

#undef WREN_KEYWORD
#undef WREN_PUNCTUATOR
#undef WREN_TOKEN
#undef WREN_TOKENDEF
