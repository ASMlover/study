//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include "el_test_header.h"
#include "../el_circular_buffer.h"



#define DATA_NUM  (256)


static inline void 
InitData(int count, short* data)
{
  srand((unsigned int)time(0));
  for (int i = 0; i < count; ++i) 
    data[i] = rand() % 10000;
}

UNIT_IMPL(CircularBufferSingle)
{
  const int BUFFER_LEN = 1024 * 1024;
  short data[DATA_NUM];
  InitData(DATA_NUM, data);
  
  el::CircularBuffer buffer;

  UNIT_ASSERT(buffer.Create());
  UNIT_ASSERT(BUFFER_LEN == buffer.length());
  UNIT_ASSERT(0 == buffer.data_length());
  UNIT_ASSERT(buffer.length() == buffer.free_length());

  UNIT_ASSERT(sizeof(data) == buffer.Write(data, sizeof(data)));
  UNIT_ASSERT(sizeof(data) == buffer.data_length());
  UNIT_ASSERT(buffer.length() - (int)sizeof(data) == buffer.free_length());

  short read_data[DATA_NUM];
  UNIT_ASSERT(sizeof(read_data) == 
      buffer.Read(sizeof(read_data), read_data));
  UNIT_ASSERT(0 == buffer.data_length());
  UNIT_ASSERT(BUFFER_LEN == buffer.free_length());
  UNIT_ASSERT(0 == memcmp(read_data, data, sizeof(data)));

  UNIT_ASSERT(sizeof(data) == buffer.Write(data, sizeof(data)));
  InitData(DATA_NUM, data);
  UNIT_ASSERT(sizeof(data) == buffer.Write(data, sizeof(data)));
  UNIT_ASSERT(2 * sizeof(data) == buffer.data_length());
  UNIT_ASSERT(sizeof(data) == buffer.Remove(sizeof(data)));
  UNIT_ASSERT(sizeof(data) == buffer.data_length());

  buffer.Clear();
  UNIT_ASSERT(0 == buffer.data_length());
  UNIT_ASSERT(BUFFER_LEN == buffer.length());
  UNIT_ASSERT(BUFFER_LEN == buffer.free_length());

  buffer.Release();
  UNIT_ASSERT(0 == buffer.length());
  UNIT_ASSERT(0 == buffer.data_length());
  UNIT_ASSERT(0 == buffer.free_length());
}
