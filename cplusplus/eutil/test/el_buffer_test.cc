// Copyright (c) 2014 ASMlover. All rights reserved.
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
#include "el_unit.h"
#include "../el_buffer.h"



UNIT_IMPL(Buffer) {
  el::Buffer buf;

  buf.Init();
  EL_ASSERT(nullptr != buf.buffer());
  EL_ASSERT(0 == buf.length());
  EL_ASSERT(buf.free_buffer() == buf.buffer());
  EL_ASSERT(el::Buffer::DEF_STORAGE == buf.free_length());

  char data[el::Buffer::DEF_STORAGE] = {0};
  memset(data, 1, sizeof(data));

  buf.Put(data, sizeof(data));
  EL_ASSERT(sizeof(data) == buf.length());
  EL_ASSERT(0 == buf.free_length());

  memset(data, 0, sizeof(data));
  buf.Get(256, data);
  EL_ASSERT(256 == buf.free_length());
  EL_ASSERT(el::Buffer::DEF_STORAGE - 256 == buf.length());

  buf.Increment(128);
  EL_ASSERT(128 == buf.free_length());

  buf.Decrement(128);
  EL_ASSERT(256 == buf.free_length());
  EL_ASSERT(el::Buffer::DEF_STORAGE - 256 == buf.length());
}
