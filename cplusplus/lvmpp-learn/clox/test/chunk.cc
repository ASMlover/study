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
#include "../harness.hh"
#include "../chunk.hh"


CLOX_TEST(CloxChunk) {
#define EXPV(v)  chunk.write_constant((v), n)
#define EXPC(c)  chunk.write((c), n)
#define EXPC2(c) chunk.write((c), n++)

  clox::Chunk chunk;
  int n = 1;

  // 22.45 + 34
  EXPV(34); EXPV(22.45); EXPC2(clox::OpCode::OP_ADD);

  // 10.45 - 87.89
  EXPV(87.89); EXPV(10.45); EXPC2(clox::OpCode::OP_SUBTRACT);

  // 78.01 * 23
  EXPV(23); EXPV(78.01); EXPC2(clox::OpCode::OP_MULTIPLY);

  // 34.91 / 82.45
  EXPV(82.45); EXPV(34.91); EXPC2(clox::OpCode::OP_DIVIDE);

  // ((98.23 - (-45.89)) + 8.19) / 78.29
  EXPV(78.29); EXPV(8.19); EXPV(-45.89); EXPV(98.23);
  EXPC(clox::OpCode::OP_SUBTRACT);
  EXPC(clox::OpCode::OP_ADD);
  EXPC2(clox::OpCode::OP_DIVIDE);

  // return
  chunk.write(clox::OpCode::OP_RETURN, n);
  chunk.dis("Clox-Chunk");

#undef EXPC2
#undef EXPC
#undef EXPV
}
