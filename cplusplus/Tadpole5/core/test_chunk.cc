// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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
#include "../common/harness.hh"
#include "chunk.hh"

TADPOLE_TEST(TadpoleChunk) {
  using TC = tadpole::Code;

  tadpole::Chunk c;
  int n = 0;

  // 89.56 + 45
  c.write_constant(45, n);
  c.write_constant(89.56, n);
  c.write(TC::ADD, n);
  ++n;

  // 90.67 * 78.56
  c.write_constant(78.56, n);
  c.write_constant(90.67, n);
  c.write(TC::MUL, n);
  ++n;

  // (10.78 - 53.2 * 1.09) / 69.93
  c.write_constant(69.93, n);
  c.write_constant(1.09, n);
  c.write_constant(53.2, n);
  c.write(TC::MUL, n);
  c.write_constant(10.78, n);
  c.write(TC::SUB, n);
  c.write(TC::DIV, n);
  ++n;

  // return
  c.write(TC::RETURN, n);

  c.dis("TadpoleChunk");
}
