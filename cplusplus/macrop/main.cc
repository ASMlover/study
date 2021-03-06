// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "scanner.h"
#include "parser.h"

static void print_tokens(const std::vector<Token>& tokens) {
  for (auto& t : tokens)
    std::cout << t.get_name() << "|"
      << t.get_lexeme() << "|" << t.get_line() << std::endl;
}

static void run_with_file(const std::string& filepath) {
  std::ifstream fp(filepath);

  if (fp.is_open()) {
    std::stringstream ss;
    ss << fp.rdbuf();

    // run(ss.str());
    Scanner s(ss.str());
    auto tokens = s.scan_tokens();
    print_tokens(tokens);
    Parser p(tokens);
    p.dump_macros();
  }
}

static void run_with_repl(void) {
  std::string line;
  for (;;) {
    std::cout << ">>> ";

    if (!std::getline(std::cin, line) || line == "exit") {
      std::cout << std::endl;
      break;
    }

    // run(line);
    Scanner s(line);
    auto tokens = s.scan_tokens();
    print_tokens(tokens);
    Parser p(tokens);
    p.dump_macros();
  }
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  if (argc < 2) {
    run_with_repl();
  }
  else if (argc == 2) {
    run_with_file(argv[1]);
  }
  else {
    std::cerr << "USAGE: " << argv[0] << " [file] ..." << std::endl;
    std::exit(1);
  }

  return 0;
}
