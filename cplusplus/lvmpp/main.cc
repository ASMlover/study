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
#include "chunk.h"
#include "vm.h"

static void chunk_sample(void) {
  {
    lox::Chunk chunk;
    lox::VM vm(chunk);

    int constant = chunk.add_constant(1.2);
    chunk.write(lox::OpCode::OP_CONSTANT, 123);
    chunk.write(constant, 123);
    chunk.write(lox::OpCode::OP_NEGATIVE, 123);
    chunk.write(lox::OpCode::OP_RETURN, 123);

    chunk.disassemble("test chunk");
    std::cout << std::endl;

    vm.interpret();
  }

  {
    lox::Chunk chunk;
    lox::VM vm(chunk);

    // test (2.45 + 4.67) / 5.6 * (67.8 - 56.3)
    // constant 2.45
    int constant = chunk.add_constant(2.45);
    chunk.write(lox::OpCode::OP_CONSTANT, 124);
    chunk.write(constant, 124);
    // constant 4.67
    constant = chunk.add_constant(4.67);
    chunk.write(lox::OpCode::OP_CONSTANT, 124);
    chunk.write(constant, 124);
    // binary +
    chunk.write(lox::OpCode::OP_ADD, 124);
    // constant 5.6
    constant = chunk.add_constant(5.6);
    chunk.write(lox::OpCode::OP_CONSTANT, 124);
    chunk.write(constant, 124);
    // binary /
    chunk.write(lox::OpCode::OP_DIVIDE, 124);
    // constant 67.8
    constant = chunk.add_constant(67.8);
    chunk.write(lox::OpCode::OP_CONSTANT, 124);
    chunk.write(constant, 124);
    // constant 56.3
    constant = chunk.add_constant(56.3);
    chunk.write(lox::OpCode::OP_CONSTANT, 124);
    chunk.write(constant, 124);
    // binary -
    chunk.write(lox::OpCode::OP_SUBTRACT, 124);
    // binary *
    chunk.write(lox::OpCode::OP_MULTIPLY, 124);
    chunk.write(lox::OpCode::OP_RETURN, 124);

    chunk.disassemble("test chunk");
    std::cout << std::endl;

    vm.interpret();
  }

  {
    lox::Chunk chunk;
    lox::VM vm(chunk);

    // 33 % 7
    // constant 33
    int constant = chunk.add_constant(33);
    chunk.write(lox::OpCode::OP_CONSTANT, 125);
    chunk.write(constant, 125);
    // constant 7
    constant = chunk.add_constant(7);
    chunk.write(lox::OpCode::OP_CONSTANT, 125);
    chunk.write(constant, 125);
    // binary %
    chunk.write(lox::OpCode::OP_MODULO, 125);
    chunk.write(lox::OpCode::OP_RETURN, 125);

    chunk.disassemble("test chunk");
    std::cout << std::endl;

    vm.interpret();
  }
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  chunk_sample();

  return 0;
}
