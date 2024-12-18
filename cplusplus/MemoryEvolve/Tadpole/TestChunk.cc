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
#include <Core/MEvolve.hh>
#include <Tadpole/Chunk.hh>

_MEVO_TEST(TadpoleChunk, _mevo::FakeTester) {
  _mevo::tadpole::Chunk c;

  // 45 + 67
  c.write(_mevo::tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(45), 0);
  c.write(_mevo::tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(67), 0);
  c.write(_mevo::tadpole::Code::ADD, 0);

  // (45 + 67) - 89
  c.write(_mevo::tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(89), 0);
  c.write(_mevo::tadpole::Code::SUB, 0);

  // ((45 + 67) - 89) * 34.56
  c.write(_mevo::tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(34.56), 0);
  c.write(_mevo::tadpole::Code::MUL, 0);

  // (((45 + 67) - 89) * 34.56) / 78.23
  c.write(_mevo::tadpole::Code::CONSTANT, 0);
  c.write(c.add_constant(78.23), 0);
  c.write(_mevo::tadpole::Code::DIV, 0);

  c.write(_mevo::tadpole::Code::RETURN, 1);

  c.dis("Tadpole.Chunk");
}
