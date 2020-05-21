#include "harness.hh"
#include "lexer.hh"

TADPOLE_TEST(TadpoleLexer) {
  using TK = tadpole::TokenKind;
#define TEST_EQ(o) TADPOLE_CHECK_EQ(lex.next_token().kind(), o)
#define DUMP_LEX() do {\
    for (;;) {\
      auto t = lex.next_token();\
      std::fprintf(stdout, "%-24s | %-32s | %d\n",\
        tadpole::get_token_name(t.kind()),\
        t.literal().c_str(),\
        t.lineno());\
      if (t.kind() == TK::TK_EOF)\
        break;\
    }\
  } while (false)

  {
    tadpole::str_t s = "print(\"Welcome to Tadpole!!!\", 45 * 34 / 23.89 + 45 - (23 - 34 + 6));";
    tadpole::Lexer lex(s);
    DUMP_LEX();
  }

  {
    tadpole::str_t s = "var a = 56;";
    tadpole::Lexer lex(s);
    TEST_EQ(TK::KW_VAR);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_EQ);
    TEST_EQ(TK::TK_NUMERIC);
    TEST_EQ(TK::TK_SEMI);
    TEST_EQ(TK::TK_EOF);
  }
  {
    tadpole::str_t s = "print(\"34 + 56 =\", 34 + 56);";
    tadpole::Lexer lex(s);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_LPAREN);
    TEST_EQ(TK::TK_STRING);
    TEST_EQ(TK::TK_COMMA);
    TEST_EQ(TK::TK_NUMERIC);
    TEST_EQ(TK::TK_PLUS);
    TEST_EQ(TK::TK_NUMERIC);
    TEST_EQ(TK::TK_RPAREN);
    TEST_EQ(TK::TK_SEMI);
    TEST_EQ(TK::TK_EOF);
  }
  {
    tadpole::str_t s = "fn show(msg) { print(msg); }";
    tadpole::Lexer lex(s);
    TEST_EQ(TK::KW_FN);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_LPAREN);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_RPAREN);
    TEST_EQ(TK::TK_LBRACE);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_LPAREN);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_RPAREN);
    TEST_EQ(TK::TK_SEMI);
    TEST_EQ(TK::TK_RBRACE);
    TEST_EQ(TK::TK_EOF);
  }
  {
    tadpole::str_t s = "print(45 * 56 + 22 / 8.9 - 100);";
    tadpole::Lexer lex(s);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_LPAREN);
    TEST_EQ(TK::TK_NUMERIC);
    TEST_EQ(TK::TK_STAR);
    TEST_EQ(TK::TK_NUMERIC);
    TEST_EQ(TK::TK_PLUS);
    TEST_EQ(TK::TK_NUMERIC);
    TEST_EQ(TK::TK_SLASH);
    TEST_EQ(TK::TK_NUMERIC);
    TEST_EQ(TK::TK_MINUS);
    TEST_EQ(TK::TK_NUMERIC);
    TEST_EQ(TK::TK_RPAREN);
    TEST_EQ(TK::TK_SEMI);
    TEST_EQ(TK::TK_EOF);
  }
  {
    tadpole::str_t s = "var a = nil; print(a, true, false);";
    tadpole::Lexer lex(s);
    TEST_EQ(TK::KW_VAR);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_EQ);
    TEST_EQ(TK::KW_NIL);
    TEST_EQ(TK::TK_SEMI);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_LPAREN);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_COMMA);
    TEST_EQ(TK::KW_TRUE);
    TEST_EQ(TK::TK_COMMA);
    TEST_EQ(TK::KW_FALSE);
    TEST_EQ(TK::TK_RPAREN);
    TEST_EQ(TK::TK_SEMI);
    TEST_EQ(TK::TK_EOF);
  }
  {
    tadpole::str_t s = "var a = \"ERROR";
    tadpole::Lexer lex(s);
    TEST_EQ(TK::KW_VAR);
    TEST_EQ(TK::TK_IDENTIFIER);
    TEST_EQ(TK::TK_EQ);
    TEST_EQ(TK::TK_ERR);
    TEST_EQ(TK::TK_EOF);
  }

#undef DUMP_LEX
#undef TEST_EQ
}