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

#include <iostream>
#include <fstream>
#include "Types.hh"
#include "Lexer.hh"
#include "Parser.hh"
#include "Interpreter.hh"

static void run_script(const ms::str_t& source_bytes) noexcept {
  ms::Lexer lex(source_bytes);
  auto tokens = lex.scan_tokens();

  ms::Parser parser(tokens);
  auto statements = parser.parse();

  ms::Interpreter interpreter;
  interpreter.interpret(statements);
}

static void run_file(const ms::str_t& path) noexcept {
  std::ifstream fp{path};
  if (!fp) {
    std::cerr << "Could not open file: " << path << std::endl;
    return;
  }

  ms::ss_t buffer;
  buffer << fp.rdbuf();
  run_script(buffer.str());
}

static void run_prompt() noexcept {
  ms::Interpreter interpreter;

  for (;;) {
    std::cout << ">>> ";
    ms::str_t line;
    if (!std::getline(std::cin, line)) {
      break;
    }

    try {
      ms::Lexer lex(line);
      auto tokens = lex.scan_tokens();

      ms::Parser parser(tokens);
      auto statements = parser.parse();

      interpreter.interpret(statements);
    }
    catch (const std::runtime_error& error) {
      std::cerr << "Error: " << error.what() << std::endl;
    }
  }
}

int main(int argc, char* argv[]) {
  MAPLE_UNUSED(argc), MAPLE_UNUSED(argv);

  std::cout << "Maple Script !!!" << std::endl;
  return 0;
}
