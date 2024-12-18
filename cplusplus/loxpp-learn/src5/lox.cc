// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <fstream>
#include <iostream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
#include "interpreter.h"
#include "lox.h"

namespace lox {

void Lox::eval(int argc, char* argv[]) {
  if (argc < 2) {
    eval_with_repl();
  }
  else if (argc == 2) {
    eval_with_file(argv[1]);
  }
  else {
    std::cerr << "USAGE: " << argv[0] << " {script}" << std::endl;
    std::exit(1);
  }
}

void Lox::eval_with_file(const std::string& fname) {
  std::ifstream fp(fname);
  if (fp.is_open()) {
    std::stringstream ss;
    ss << fp.rdbuf();

    run(ss.str(), fname);
  }
}

void Lox::eval_with_repl(void) {
  std::string line;
  for (;;) {
    std::cout << ">>> ";

    if (!std::getline(std::cin, line) || line == "exit") {
      break;
    }
    line += "\n";
    run(line);
  }
}

void Lox::run(const std::string& sources, const std::string& fname) {
  Lexer lex(err_report_, sources, fname);
  const auto& tokens = lex.parse_tokens();
  if (err_report_.had_error())
    std::abort();

  for (auto& tok : tokens)
    std::cout << tok << std::endl;

  //////////////////// OLD INTERPRETER TEST ////////////////////
  // Parser parser(err_report_, tokens);
  // auto expr = parser.parse();
  // auto astp = std::make_shared<AstPrinter>();
  // if (err_report_.had_error())
  //   std::abort();

  // std::cout << astp->stringify(expr) << std::endl;

  // auto interp = std::make_shared<Interpreter>(err_report_);
  // interp->interpret(expr);
  // if (err_report_.had_error())
  //   std::abort();
  //////////////////////////////////////////////////////////////

  Parser parser(err_report_, tokens);
  auto stmts = parser.parse_stmt();
  if (err_report_.had_error())
    std::abort();

  auto interp = std::make_shared<Interpreter>(err_report_);
  interp->interpret(stmts);
  if (err_report_.had_error())
    std::abort();
}

}
