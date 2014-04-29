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
#include "el_util.h"
#include "el_buffer.h"



namespace el {

Buffer::Buffer(void) 
  : buffer_(nullptr)
  , length_(0)
  , storage_(DEF_STORAGE) {
}

Buffer::~Buffer(void) {
  Destroy();
}

bool Buffer::Init(int storage) {
  storage_ = MAX(storage, DEF_STORAGE);

  length_ = 0;
  buffer_ = static_cast<char*>(malloc(storage_));
  EL_ASSERT(nullptr != buffer_);

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

int Buffer::Put(const char* buffer, int bytes) {
  if (length_ + bytes > storage_)
    Regrow();

  memcpy(buffer_ + length_, buffer, bytes);
  length_ += bytes;

  return bytes;
}

int Buffer::Get(int bytes, char* buffer) {
  int copy_bytes = MIN(length_, bytes);
  if (0 == copy_bytes)
    return 0;

  memcpy(buffer, buffer_, copy_bytes);
  if (copy_bytes != length_)
    memmove(buffer_, buffer + copy_bytes, length_ - copy_bytes);
  length_ -= copy_bytes;

  return copy_bytes;
}

int Buffer::Increment(int bytes) {
  if (length_ + bytes > storage_)
    Regrow();

  length_ += bytes;
  return bytes;
}

int Buffer::Decrement(int bytes) {
  EL_ASSERT(length_ >= bytes);

  if (bytes < length_)
    memmove(buffer_, buffer_ + bytes, length_ - bytes);
  length_ -= bytes;

  return bytes;
}

void Buffer::Regrow(void) {
  int new_storage = MAX(2 * storage_, DEF_STORAGE);
  buffer_ = static_cast<char*>(realloc(buffer_, new_storage));
  EL_ASSERT(nullptr != buffer_);

  storage_ = new_storage;
}

}
