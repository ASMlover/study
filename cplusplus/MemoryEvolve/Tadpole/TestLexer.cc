#include <Core/MEvolve.hh>
#include <Tadpole/Lexer.hh>

_MEVO_TEST(TadpoleLexer, _mevo::FakeTester) {
  auto token_repr = [](const _mevo::tadpole::Token& tok) {
    std::fprintf(stdout, "%-16s|%4d|%s\n",
      _mevo::tadpole::get_token_name(tok.kind()),
      tok.line(),
      tok.literal().c_str());
  };

  _mevo::str_t source_bytes =
    "// test case for Lexer\n"
    "var a = 34;\n"
    "var b = 56.43;\n"
    "var c = (a + b) * a / b;\n"
    "print(\"calculate value is: \", c);\n"
    ;
  _mevo::tadpole::Lexer lex(source_bytes);
  for (;;) {
    auto tok = lex.next_token();
    token_repr(tok);

    if (tok.kind() == _mevo::tadpole::TokenKind::TK_EOF)
      break;
  }
}