#include "common.hh"
#include "harness.hh"
#include "token.hh"
import common;
import harness;
import token;

TADPOLE_TEST(TadpoleToken) {
  using TK = tadpole::TokenKind;

#define NEWTK2(k, s)    tadpole::Token(k, s)
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

#undef TESTSTR
#undef TESTTK
#undef TESTEQ
#undef NEWTK3
#undef NEWTK2
}