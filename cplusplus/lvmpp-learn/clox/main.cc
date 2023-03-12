// Copyright (c) 2023 ASMlover. All rights reserved.
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
#include <string>
#include "common.hh"
#include "harness.hh"
#include "chunk.hh"
#include "vm.hh"

static clox::VM& get_vm() noexcept {
  static clox::VM vm;
  return vm;
}

static void repl() noexcept {
  clox::str_t line;
  for (;;) {
    std::cout << "> ";

    if (!std::getline(std::cin, line))
      break;

    get_vm().interpret(line);
  }
}

static void run_file(const char* filepath) noexcept {
  std::fstream fp(filepath);
  if (!fp.is_open()) {
    std::cerr << "ERROR LOAD: `" << filepath << "` FAILED !!!" << std::endl;
    std::exit(74);
  }

  clox::ss_t ss;
  ss << fp.rdbuf();

  clox::InterpretResult result = get_vm().interpret(ss.str());
  if (result == clox::InterpretResult::INTERPRET_COMPILE_ERROR)
    std::exit(65);
  if (result == clox::InterpretResult::INTERPRET_RUNTIME_ERROR)
    std::exit(70);
}

int main(int argc, char* argv[]) {
  CLOX_UNUSED(argc), CLOX_UNUSED(argv);

#if defined(_CLOX_RUN_HARNESS)
  clox::harness::run_all_harness();
#endif

  clox::VM& vm = get_vm();

  if (argc == 1) {
    repl();
  }
  else if (argc == 2) {
    run_file(argv[1]);
  }
  else {
    std::cerr << "Usage: clox [filepath]" << std::endl;
    std::exit(64);
  }

  clox::Chunk chunk;

  // (1.2 + 3.4) / 5.6
  chunk.write_constant(1.2, 1);
  chunk.write_constant(3.4, 1);
  chunk.write(clox::OpCode::OP_ADD, 1);
  chunk.write_constant(5.6, 1);
  chunk.write(clox::OpCode::OP_DIVIDE, 1);

  chunk.write(clox::OpCode::OP_NEGATE, 1);
  chunk.write(clox::OpCode::OP_RETURN, 1);

  vm.interpret(&chunk);

  return 0;
}
