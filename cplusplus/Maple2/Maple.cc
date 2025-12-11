// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include "Scanner.hh"
#include "Parser.hh"
#include "Resolver.hh"
#include "Interpreter.hh"
#include "Maple.hh"

namespace ms {

int Maple::run(int argc, char* argv[]) {
  MAPLE_UNUSED(argc), MAPLE_UNUSED(argv);

  if (argc > 2)
    std::cout << "Usage: maple [script]" << std::endl;
  else if (argc == 2)
    run_from_file(argv[1]);
  else
    run_from_prompt();

  return 0;
}

void Maple::run_from_file(const str_t& filepath) noexcept {
  if (std::ifstream fp(filepath); fp.is_open()) {
    ss_t ss;
    ss << fp.rdbuf();

    run(filepath, ss.str());
  }
}

void Maple::run_from_prompt() noexcept {
  str_t line;
  for (;;) {
    std::cout << ">>> ";
    if (!std::getline(std::cin, line) || line == "exit")
      break;

    run("", line);
  }
}

void Maple::run(const str_t& filepath, const str_t& source_bytes) noexcept {
  Scanner scanner{err_reporter_, source_bytes, ""};
  const auto& tokens = scanner.scan_tokens();

  Parser parser{err_reporter_, tokens};
  auto statements = parser.parse();

  auto interp = std::make_shared<Interpreter>(err_reporter_);
  auto resolver = std::make_shared<Resolver>(err_reporter_, interp);
  resolver->invoke_resolve(statements);

  if (err_reporter_.had_error())
    return;

  interp->interpret(statements);
}

}
