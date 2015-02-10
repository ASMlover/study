// Copyright (c) 2015 ASMlover. All rights reserved.
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
#include "el_poll.h"
#include "el_buffer.h"

namespace el {

Buffer::Buffer(void)
  : buffer_(nullptr)
  , length_(0)
  , storage_(0) {
}

Buffer::~Buffer(void) {
  Destroy();
}

bool Buffer::Init(uint32_t storage) {
  storage_ = (storage > DEF_STORAGE ? storage : DEF_STORAGE);

  length_ = 0;
  buffer_ = (char*)malloc(storage_);
  EL_ASSERT(nullptr != buffer_, "init buffer failed ...");

  return true;
}

void Buffer::Destroy(void) {
  if (nullptr != buffer_) {
    free(buffer_);
    buffer_ = nullptr;
  }
  length_ = 0;
  storage_ = DEF_STORAGE;
}

uint32_t Buffer::Put(const char* buffer, uint32_t bytes) {
  if (length_ + bytes > storage_) {
    if (!Regrow(DEF_STORAGE))
      return 0;
  }

  memcpy(buffer_ + length_, buffer, bytes);
  length_ += bytes;

  return bytes;
}

uint32_t Buffer::Get(uint32_t bytes, char* buffer) {
  uint32_t copy_bytes = length_ <= bytes ? length_ : bytes;
  if (0 == copy_bytes)
    return 0;

  memcpy(buffer, buffer_, copy_bytes);
  if (copy_bytes != length_)
    memmove(buffer_, buffer_ + copy_bytes, length_ - copy_bytes);
  length_ -= copy_bytes;

  return copy_bytes;
}

uint32_t Buffer::Inc(uint32_t bytes) {
  if (length_ + bytes > storage_) {
    if (!Regrow(DEF_STORAGE))
      return 0;
  }
  length_ += bytes;

  return bytes;
}

uint32_t Buffer::Dec(uint32_t bytes) {
  EL_ASSERT(bytes > length_, "get bytes > buffer bytes ...");

  if (bytes < length_)
    memcpy(buffer_, buffer_ + bytes, length_ - bytes);
  length_ -= bytes;

  return bytes;
}

bool Buffer::Regrow(uint32_t regrow_length) {
  uint32_t new_storage = (0 != regrow_length 
     ? storage_ + regrow_length 
     : (0 != storage_ ? 2 * storage_ : DEF_STORAGE));
  buffer_ = (char*)realloc(buffer_, new_storage);
  EL_ASSERT(nullptr != buffer_, "regrow buffer failed ...");

  storage_ = new_storage;
  return true;
}

}
