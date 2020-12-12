#include "common.hh"
#include "harness.hh"
#include "token.hh"
import common;
import harness;
import token;

TADPOLE_TEST(TadpoleToken) {
  using TK = tadpole::TokenKind;

#define NEWTK2(k, s)    tadpole::Token(k, s, 0)
#define NEWTK3(k, s, n) tadpole::Token(k, s, n)
#define TESTEQ(a, b)    TADPOLE_CHECK_EQ(a, b)
#define TESTTK(k, s) do {\
    auto t = NEWTK2(k, s);\
    TESTEQ(t.kind(), k);\
    TESTEQ(t.literal(), s);\
    TESTEQ(t.lineno(), 0);\
  } while (false)
#define TESTSTR(s, n) do {\
    auto t = NEWTK3(TK::TK_STRING, s, n);\
    TESTEQ(t.kind(), TK::TK_STRING);\
    TESTEQ(t.literal(), s);\
    TESTEQ(t.lineno(), n);\
  } while (false)

  TESTTK(TK::TK_LPAREN, "(");
  TESTTK(TK::TK_RPAREN, ")");
  TESTTK(TK::TK_LBRACE, "{");
  TESTTK(TK::TK_RBRACE, "}");
  TESTTK(TK::TK_COMMA, ",");
  TESTTK(TK::TK_MINUS, "-");
  TESTTK(TK::TK_PLUS, "+");
  TESTTK(TK::TK_SEMI, ";");
  TESTTK(TK::TK_SLASH, "/");
  TESTTK(TK::TK_STAR, "*");
  TESTTK(TK::TK_EQ, "=");

  TESTTK(TK::KW_FALSE, "false");
  TESTTK(TK::KW_FN, "fn");
  TESTTK(TK::KW_NIL, "nil");
  TESTTK(TK::KW_TRUE, "true");
  TESTTK(TK::KW_VAR, "var");

  TESTTK(TK::TK_EOF, "");
  TESTTK(TK::TK_ERR, "");

  // test for identifier
  TESTTK(TK::TK_IDENTIFIER, "foo");
  TESTTK(TK::TK_IDENTIFIER, "bar");
  TESTTK(TK::TK_IDENTIFIER, "Count");
  TESTTK(TK::TK_IDENTIFIER, "_Count");
  TESTTK(TK::TK_IDENTIFIER, "var1");
  TESTTK(TK::TK_IDENTIFIER, "var2");

  {
    // test for numeric
    auto t1 = NEWTK2(TK::TK_NUMERIC, "100");
    TESTEQ(t1.kind(), TK::TK_NUMERIC);
    TESTEQ(t1.as_numeric(), 100);
    TESTEQ(t1.lineno(), 0);

    auto t2 = NEWTK2(TK::TK_NUMERIC, "3.14");
    TESTEQ(t2.kind(), TK::TK_NUMERIC);
    TESTEQ(t2.as_numeric(), 3.14);
    TESTEQ(t2.lineno(), 0);
  }

  // test for string
  TESTSTR("Hello", 5);
  TESTSTR("XXX", 3);
  TESTSTR("String for token", 16);

#undef TESTSTR
#undef TESTTK
#undef TESTEQ
#undef NEWTK3
#undef NEWTK2
}