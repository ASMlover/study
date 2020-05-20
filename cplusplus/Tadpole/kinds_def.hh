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

TOKEN(IDENTIFIER, "Tadpole-Identifier")
TOKEN(NUMERIC, "Tadpole-Numeric")
TOKEN(STRING, "Tadpole-String")

KEYWORD(FALSE, "false")
KEYWORD(FN, "fn")
KEYWORD(NIL, "nil")
KEYWORD(TRUE, "true")
KEYWORD(VAR, "var")

TOKEN(EOF, "Tadpole-EOF")
TOKEN(ERR, "Tadpole-ERR")

#undef KEYWORD
#undef PUNCTUATOR
#undef TOKEN
#undef TOKDEF