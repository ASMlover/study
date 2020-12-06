#if !defined(TOKDEF)
# define TOKDEF(k, s)
#endif

#if !defined(TOKEN)
# define TOKEN(k, s) TOKDEF(TK_##k, s)
#endif

#if !defined(PUNCTUATOR)
# define PUNCTUATOR(k, s) TOKEN(k, s)
#endif

#if !defined(KEYWORD)
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

TOKEN(IDENTIFIER, "Identifier")
TOKEN(NUMERIC, "Numeric")
TOKEN(STRING, "String")

KEYWORD(FALSE, "false")
KEYWORD(FN, "fn")
KEYWORD(NIL, "nil")
KEYWORD(TRUE, "true")
KEYWORD(VAR, "var")

TOKEN(EOF, "Eof")
TOKEN(ERR, "Error")

#undef KEYWORD
#undef PUNCTUATOR
#undef TOKEN
#undef TOKDEF