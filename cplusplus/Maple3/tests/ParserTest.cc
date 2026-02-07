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
#include "Parser.hh"
#include "Scanner.hh"

namespace {

bool expect(bool condition, const ms::str_t& message) {
  if (!condition)
    std::cerr << "[FAIL] " << message << std::endl;
  return condition;
}

ms::ExprPtr parse_expression_from(const ms::str_t& source, ms::str_t* error = nullptr) {
  ms::Scanner scanner{source, "parser_expr.ms"};
  auto tokens = scanner.scan_tokens();

  ms::Parser parser{tokens, "parser_expr.ms"};
  auto expr = parser.parse_expression();
  if (error != nullptr && parser.has_error())
    *error = parser.errors().front();
  return expr;
}

bool test_precedence() {
  auto expr = parse_expression_from("1 + 2 * 3");
  bool ok = true;
  ok = expect(expr != nullptr, "precedence expression should parse") && ok;
  ok = expect(ms::debug_expr(expr.get()) == "(+ 1 (* 2 3))", "operator precedence should be correct") && ok;
  return ok;
}

bool test_grouping() {
  auto expr = parse_expression_from("(1 + 2) * 3");
  bool ok = true;
  ok = expect(expr != nullptr, "grouping expression should parse") && ok;
  ok = expect(ms::debug_expr(expr.get()) == "(* (group (+ 1 2)) 3)", "grouping should bind before multiply") && ok;
  return ok;
}

bool test_program_recovery() {
  ms::Scanner scanner{"1 + ; 2 + 3;", "parser_program.ms"};
  auto tokens = scanner.scan_tokens();

  ms::Parser parser{tokens, "parser_program.ms"};
  auto exprs = parser.parse_program();

  bool ok = true;
  ok = expect(parser.has_error(), "invalid program should contain parse error") && ok;
  ok = expect(exprs.size() == 1, "parser should recover and keep valid trailing expression") && ok;
  if (!exprs.empty())
    ok = expect(ms::debug_expr(exprs[0].get()) == "(+ 2 3)", "recovered expression should match trailing statement") && ok;
  return ok;
}

}

int main() {
  bool ok = true;
  ok = test_precedence() && ok;
  ok = test_grouping() && ok;
  ok = test_program_recovery() && ok;

  if (ok) {
    std::cout << "[PASS] Parser tests" << std::endl;
    return 0;
  }
  return 1;
}
