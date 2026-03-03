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
#include "Chunk.hh"
#include "Debug.hh"
#include "Logger.hh"

static void repl() noexcept {
  ms::str_t line;
  for (;;) {
    std::cout << "maple> ";

    if (!std::getline(std::cin, line)) {
      std::cout << std::endl;
      break;
    }

    // TODO: pass line to VM for interpretation
    std::cout << line << std::endl;
  }
}

static void run_file(const ms::str_t& path) noexcept {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Could not open file \"" << path << "\"." << std::endl;
    std::exit(74);
  }

  // TODO: read source and pass to VM for interpretation
  std::cout << "Running: " << path << std::endl;
}

static void phase1_test() noexcept {
  ms::Chunk chunk;

  // Add constants
  auto idx_a = chunk.add_constant(ms::Value(1.2));
  auto idx_b = chunk.add_constant(ms::Value(3.4));

  // Write instructions: OP_CONSTANT(0), OP_CONSTANT(1), OP_ADD, OP_NEGATE, OP_PRINT, OP_RETURN
  chunk.write(ms::OpCode::OP_CONSTANT, 1);
  chunk.write(static_cast<ms::u8_t>(idx_a), 1);

  chunk.write(ms::OpCode::OP_CONSTANT, 1);
  chunk.write(static_cast<ms::u8_t>(idx_b), 1);

  chunk.write(ms::OpCode::OP_ADD, 1);
  chunk.write(ms::OpCode::OP_NEGATE, 1);
  chunk.write(ms::OpCode::OP_PRINT, 1);
  chunk.write(ms::OpCode::OP_RETURN, 2);

  // Disassemble
  ms::disassemble_chunk(chunk, "phase1-test");

  // Logger test messages
  auto& logger = ms::Logger::get_instance();
  logger.set_level(ms::LogLevel::TRACE);
  logger.trace("main", "Phase 1 trace message");
  logger.debug("main", "Phase 1 debug message");
  logger.info("main", "Phase 1 verification complete");
  logger.warn("main", "VM not yet implemented");
  logger.error("main", "This is a sample error (not a real error)");
}

int main(int argc, char* argv[]) {
  phase1_test();

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
