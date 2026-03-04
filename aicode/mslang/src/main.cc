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
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include "VM.hh"

static void repl() noexcept {
  auto& vm = ms::VM::get_instance();
  ms::str_t line;

  for (;;) {
    std::cout << "maple> ";

    if (!std::getline(std::cin, line)) {
      std::cout << std::endl;
      break;
    }

    if (line.empty()) continue;

    vm.interpret(line);
  }
}

static void run_file(const ms::str_t& path) noexcept {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Could not open file \"" << path << "\"." << std::endl;
    std::exit(74);
  }

  file.seekg(0, std::ios::end);
  auto size = file.tellg();
  file.seekg(0, std::ios::beg);
  ms::str_t source;
  source.resize(static_cast<ms::sz_t>(size));
  file.read(source.data(), size);

  auto& vm = ms::VM::get_instance();
  ms::InterpretResult result = vm.interpret(source);

  if (result == ms::InterpretResult::INTERPRET_COMPILE_ERROR) std::exit(65);
  if (result == ms::InterpretResult::INTERPRET_RUNTIME_ERROR) std::exit(70);
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    run_file(argv[1]);
  } else {
    std::cerr << "Usage: mslang [script]" << std::endl;
    std::exit(64);
  }

  return 0;
}
