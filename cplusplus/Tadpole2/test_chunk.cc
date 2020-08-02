// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include "harness.hh"
#include "chunk.hh"

TADPOLE_TEST(TadpoleChunk) {
  tadpole::Chunk c;

  // 34 + 56
  c.write_constant(56, 0);
  c.write_constant(34, 0);
  c.write(tadpole::Code::ADD, 0);

  // 89 * 23.45
  c.write_constant(23.45, 1);
  c.write_constant(89, 1);
  c.write(tadpole::Code::MUL, 1);

  // (34 - 565 * 56) / 89.65
  c.write_constant(89.65, 2);
  c.write_constant(56, 2);
  c.write_constant(565, 2);
  c.write(tadpole::Code::MUL, 2);
  c.write_constant(34, 2);
  c.write(tadpole::Code::SUB, 2);
  c.write(tadpole::Code::DIV, 2);

  // 1 + 45 * 23.7 / 23.67 - (89 + 22.1)
  c.write_constant(22.1, 3);
  c.write_constant(89, 3);
  c.write(tadpole::Code::ADD, 3);
  c.write_constant(23.67, 3);
  c.write_constant(23.7, 3);
  c.write_constant(45, 3);
  c.write(tadpole::Code::MUL, 3);
  c.write(tadpole::Code::DIV, 3);
  c.write_constant(1, 3);
  c.write(tadpole::Code::ADD, 3);
  c.write(tadpole::Code::SUB, 3);

  // 29 * (45 - 67.4) / (78 + 92)
  c.write_constant(92, 4);
  c.write_constant(78, 4);
  c.write(tadpole::Code::ADD, 4);
  c.write_constant(67.4, 4);
  c.write_constant(45, 4);
  c.write(tadpole::Code::SUB, 4);
  c.write_constant(29, 4);
  c.write(tadpole::Code::MUL, 4);
  c.write(tadpole::Code::DIV, 4);

  c.write(tadpole::Code::RETURN, 5);

  c.dis("TadpoleChunk");
}
