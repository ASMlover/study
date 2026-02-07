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
#include <iostream>
#include "Common.hh"
#include "Scanner.hh"

namespace {

bool expect(bool condition, const ms::str_t& message) {
  if (!condition)
    std::cerr << "[FAIL] " << message << std::endl;
  return condition;
}

bool test_scans_basic_tokens() {
  ms::str_t source = R"(
var name = "Maple\nTree";
var num = 12.34;
print name + "!";
)";

  ms::Scanner scanner{source, "scanner_basic.ms"};
  auto tokens = scanner.scan_tokens();

  bool ok = true;
  ok = expect(tokens.size() > 6, "basic token count should be > 6") && ok;
  ok = expect(tokens[0].type() == ms::TokenType::KW_VAR, "token[0] should be 'var' keyword") && ok;
  ok = expect(tokens[0].lineno() == 2 && tokens[0].colno() == 1, "token[0] should be at line 2, col 1") && ok;
  ok = expect(tokens[3].type() == ms::TokenType::TK_STRING, "token[3] should be string literal") && ok;
  ok = expect(tokens[3].as_string().find('\n') != ms::str_t::npos, "string literal should contain escaped newline") && ok;
  ok = expect(tokens.back().type() == ms::TokenType::TK_EOF, "last token should be EOF") && ok;
  return ok;
}

bool test_reports_unterminated_string() {
  ms::str_t source = "var s = \"unterminated";
  ms::Scanner scanner{source, "scanner_error.ms"};
  auto tokens = scanner.scan_tokens();

  bool has_error = false;
  for (const auto& token : tokens) {
    if (token.type() == ms::TokenType::TK_ERR) {
      has_error = token.literal().find("scanner_error.ms:1:9 error: Unterminated string.") != ms::str_t::npos;
      break;
    }
  }
  return expect(has_error, "unterminated string should produce file/line/col diagnostic");
}

bool test_reports_unterminated_block_comment() {
  ms::str_t source = "/* not closed";
  ms::Scanner scanner{source, "comment_error.ms"};
  auto tokens = scanner.scan_tokens();

  bool has_error = false;
  for (const auto& token : tokens) {
    if (token.type() == ms::TokenType::TK_ERR) {
      has_error = token.literal().find("comment_error.ms:1:1 error: Unterminated block comment.") != ms::str_t::npos;
      break;
    }
  }
  return expect(has_error, "unterminated block comment should produce diagnostic");
}

}

int main() {
  bool ok = true;
  ok = test_scans_basic_tokens() && ok;
  ok = test_reports_unterminated_string() && ok;
  ok = test_reports_unterminated_block_comment() && ok;

  if (ok) {
    std::cout << "[PASS] Scanner tests" << std::endl;
    return 0;
  }
  return 1;
}
