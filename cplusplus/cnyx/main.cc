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
#include "lexer.hh"
#include "vm.hh"
#include "compile.hh"

static void test_lexer(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "USAGE: " << argv[0] << " {FILE_NAME}" << std::endl;
    return;
  }

  std::ifstream fp(argv[1]);
  if (!fp.is_open()) {
    std::cerr << "open file \"" << argv[1] << "\" failed" << std::endl;
    return;
  }
  std::stringstream ss;
  ss << fp.rdbuf();
  std::string source_bytes(ss.str());

  nyx::Lexer lex(source_bytes);
  while (true) {
    auto tok = lex.next_token();
    std::cout << tok << std::endl;

    if (tok.get_kind() == nyx::TokenKind::TK_EOF)
      break;
  }
}

static void test_vm(void) {
  nyx::VM vm;
  // vm.interpret("1+2*3/4-5;");
  // vm.interpret("true;");
  // vm.interpret("false;");
  // vm.interpret("\"abc\" + \"123\";");
  vm.interpret("var a = 123; a = 456; a = a + a; a;");
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  std::cout << "Welcome to NYX !" << std::endl;
  // test_lexer(argc, argv);
  test_vm();

  return 0;
}
