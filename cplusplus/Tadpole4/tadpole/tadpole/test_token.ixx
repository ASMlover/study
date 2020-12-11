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

#undef TESTSTR
#undef TESTTK
#undef TESTEQ
#undef NEWTK3
#undef NEWTK2
}