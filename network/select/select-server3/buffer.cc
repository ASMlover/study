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
#include <stdlib.h>
#include <string.h>
#include "logging.h"
#include "buffer.h"



Buffer::Buffer(void)
  : buffer_(NULL)
  , storage_(0)
  , pos_(0)
{
}

Buffer::~Buffer(void)
{
  Destroy();
}

bool 
Buffer::Init(int storage)
{
  if (storage <= 0) {
    LOG_FAIL("init storage failed\n");
    return false;
  }

  pos_ = 0;
  storage_ = storage;
  buffer_ = (char*)malloc(storage_);

  return (NULL != buffer_);
}

void 
Buffer::Destroy(void)
{
  if (NULL != buffer_) {
    free(buffer_);
    buffer_ = NULL;
  }
  storage_ = 0;
  pos_ = 0;
}

int 
Buffer::Put(const char* buffer, int length)
{
  if (pos_ + length > storage_)
    Regrow();

  memcpy(buffer_ + pos_, buffer, length);
  pos_ += length;

  return length;
}

int 
Buffer::Get(int length, char* buffer)
{
  int copy_bytes = pos_ <= length ? pos_ : length;
  if (0 == copy_bytes)
    return 0;

  memcpy(buffer, buffer_, copy_bytes);
  if (copy_bytes != pos_) 
    memmove(buffer_, buffer_ + copy_bytes, pos_ - copy_bytes);

  pos_ -= copy_bytes;
  return copy_bytes;
}

void 
Buffer::Increment(int bytes)
{
  pos_ += bytes;
}

void 
Buffer::Decrement(int bytes)
{
  if (bytes > pos_) {
    LOG_FAIL("bytes failed\n");
    return;
  }

  if (bytes < pos_)
    memmove(buffer_, buffer_ + bytes, pos_ - bytes);

  pos_ -= bytes;
}


bool 
Buffer::Regrow(void)
{
  int new_storage = storage_ + kDefaultStorage;

  buffer_ = (char*)realloc(buffer_, new_storage);
  if (NULL == buffer_) {
    LOG_FAIL("realloc failed\n");
    return false;
  }

  storage_ = new_storage;
  return true;
}
