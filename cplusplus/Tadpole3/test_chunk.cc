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
  using TC = tadpole::Code;

  tadpole::Chunk c;
  int n = 0;

  // 29.78 + 45
  c.write_constant(45, n);
  c.write_constant(29.78, n);
  c.write(TC::ADD, n);
  ++n;

  // 29.78 * 33.56
  c.write_constant(33.56, n);
  c.write_constant(29.78, n);
  c.write(TC::MUL, n);
  ++n;

  // (23.67 - 33.1 * 0.78) / 78.56
  c.write_constant(78.56, n);
  c.write_constant(0.78, n);
  c.write_constant(33.1, n);
  c.write(TC::MUL, n);
  c.write_constant(23.67, n);
  c.write(TC::SUB, n);
  c.write(TC::DIV, n);
  ++n;

  // return
  c.write(TC::RETURN, n);

  c.dis("TadpoleChunk");
}
