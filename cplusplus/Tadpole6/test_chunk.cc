// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
  int n = 1;

  // 34.89 + 56
  c.write_constant(56, n);
  c.write_constant(34.89, n);
  c.write(TC::ADD, n);
  ++n;

  // 78 - 90.34
  c.write_constant(90.34, n);
  c.write_constant(78, n);
  c.write(TC::SUB, n);
  ++n;

  // 77.23 * 90.81
  c.write_constant(90.81, n);
  c.write_constant(77.23, n);
  c.write(TC::MUL, n);
  ++n;

  // 0.78 / 24.07
  c.write_constant(24.07, n);
  c.write_constant(0.78, n);
  c.write(TC::DIV, n);
  ++n;

  // (46 - 97.56 + 83.9) / 99.62
  c.write_constant(99.62, n);
  c.write_constant(83.9, n);
  c.write_constant(97.56, n);
  c.write_constant(46, n);
  c.write(TC::SUB, n);
  c.write(TC::ADD, n);
  c.write(TC::DIV, n);
  ++n;

  // 34 + 78.34 * 0.23 / 61 - (21.78 + 99)
  c.write_constant(99, n);
  c.write_constant(21.78, n);
  c.write(TC::ADD, n);
  c.write_constant(61, n);
  c.write_constant(0.23, n);
  c.write_constant(78.34, n);
  c.write(TC::MUL, n);
  c.write(TC::DIV, n);
  c.write_constant(34, n);
  c.write(TC::ADD, n);
  c.write(TC::SUB, n);
  ++n;

  // return
  c.write(TC::RETURN, n);

  c.dis("TadpoleChunk");
}
