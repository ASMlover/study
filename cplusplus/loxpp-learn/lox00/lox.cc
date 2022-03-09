// Copyright (c) 2022 ASMlover. All rights reserved.
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
#include "lox.hh"

namespace loxpp {

int Lox::run(int argc, char* argv[]) {
  LOXPP_UNUSED(argc), LOXPP_UNUSED(argv);

  if (argc > 1) {
    std::cout << "Usage: loxpp [script]" << std::endl;
  }
  else if (argc == 1) {
    run_from_file(argv[0]);
  }
  else {
    run_from_prompt();
  }

  return 0;
}

void Lox::run_from_file(const str_t& filepath) {
  if (std::ifstream fp(filepath); fp.is_open()) {
    ss_t ss;
    ss << fp.rdbuf();

    run(filepath, ss.str());
  }
}

void Lox::run_from_prompt() {
  str_t line;
  for (;;) {
    std::cout << ">>> ";
    if (!std::getline(std::cin, line) || line == "exit")
      break;

    run("", line);
  }
}

void Lox::run(const str_t& filepath, const str_t& source_bytes) {
}

}
