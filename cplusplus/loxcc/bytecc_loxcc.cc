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
#include "bytecc_vm.hh"
#include "bytecc_loxcc.hh"

namespace loxcc::bytecc {

Loxcc::Loxcc(void) noexcept
  : vm_(new VM()) {
}

void Loxcc::eval(int argc, char** argv) {
  if (argc == 2) {
    eval_with_repl();
  }
  else if (argc == 3) {
    eval_with_file(argv[2]);
  }
  else {
    std::cerr << "USAGE: " << argv[0] << " [i|c] [FILE_NAME]" << std::endl;
    std::exit(-1);
  }
}

void Loxcc::eval_with_repl(void) {
  str_t line;
  for (;;) {
    std::cout << ">>> ";

    if (!std::getline(std::cin, line) || line == "exit")
      break;

    eval(line);
  }
}

void Loxcc::eval_with_file(const str_t& fname) {
  std::fstream fp(fname);
  if (fp.is_open()) {
    std::stringstream ss;
    ss << fp.rdbuf();

    eval(ss.str());
  }
}

void Loxcc::eval(const str_t& source_bytes) {
  InterpretRet r = vm_->interpret(source_bytes);
  if (r == InterpretRet::COMPILE_ERR)
    std::exit(-2);
  if (r == InterpretRet::RUNTIME_ERR)
    std::exit(-3);
}

}
