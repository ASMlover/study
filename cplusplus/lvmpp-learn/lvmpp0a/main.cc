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
#include <iostream>
#include <fstream>
#include <sstream>
#include "chunk.hh"
#include "vm.hh"

static void use_disassembler(void) {
  lvm::Chunk chunk;
  lvm::VM vm(chunk);

  chunk.write_constant(3.12, 100);
  chunk.write_constant(2.05, 100);
  chunk.write(lvm::OpCode::OP_ADD, 100);
  chunk.write_constant(5.03, 100);
  chunk.write(lvm::OpCode::OP_DIV, 100);

  chunk.write(lvm::OpCode::OP_NEGATE, 100);
  chunk.write(lvm::OpCode::OP_RETURN, 100);

  vm.interpret();
}

static void eval_with_repl(lvm::VM& vm) {
  std::string line;
  for (;;) {
    std::cout << ">>> ";
    if (!std::getline(std::cin, line) || line == "exit")
      break;

    vm.interpret(line);
  }
}

static void eval_with_file(lvm::VM& vm, const std::string& fname) {
  std::ifstream fp(fname);
  if (fp.is_open()) {
    std::stringstream ss;
    ss << fp.rdbuf();

    auto r = vm.interpret(ss.str());
    if (r == lvm::InterpretRet::COMPILE_ERROR)
      std::exit(65);
    else if (r == lvm::InterpretRet::RUNTIME_ERROR)
      std::exit(70);
  }
  else {
    std::cerr << "could not open file \"" << fname << "\"" << std::endl;
    std::exit(74);
  }
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

#if defined(LVM_DEBUG_DIS)
  use_disassembler();
#endif

  lvm::Chunk chunk;
  lvm::VM vm(chunk);

  if (argc == 1) {
    eval_with_repl(vm);
  }
  else if (argc == 2) {
    eval_with_file(vm, argv[1]);
  }
  else {
    std::cerr << "USAGE: " << argv[0] << " {FILE_PATH}" << std::endl;
    std::exit(1);
  }

  return 0;
}
