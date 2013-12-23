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
#include "elib_internal.h"
#include "el_circular_buffer.h"


namespace el {


//! (read_pos/write_pos)
//!     |
//!     V
//!    |-----------------------------------------------------|
//!
//!            (read_pos)                      (write_pos)
//!               |                                |
//!               V                                V
//!    |----------=================================----------|
//!
//!            (write_pos)                     (read_pos)
//!               |                                |
//!               V                                V
//!    |==========---------------------------------==========|
//!
//!                     (read_pos/write_pos)
//!                             |
//!                             V
//!    |=====================================================|


CircularBuffer::CircularBuffer(void)
  : buffer_(NULL)
  , length_(0)
  , rpos_(0)
  , wpos_(0)
  , data_length_(0)
  , free_length_(0)
{
}

CircularBuffer::~CircularBuffer(void)
{
  Release();
}

bool 
CircularBuffer::Create(int length)
{
  if (NULL != buffer_)
    free(buffer_);

  length_ = (length < kDefBufferLength ? kDefBufferLength : length);
  buffer_ = (char*)malloc(length_);
  if (NULL == buffer_)
    return false;

  rpos_ = wpos_ = 0;
  data_length_ = 0;
  free_length_ = length_;

  return true;
}

void 
CircularBuffer::Release(void)
{
  if (NULL != buffer_) {
    free(buffer_);
    buffer_ = NULL;
  }
  length_ = 0;
  rpos_ = wpos_ = 0;
  data_length_ = free_length_ = 0;
}

void 
CircularBuffer::Clear(void)
{
  rpos_ = wpos_ = 0;
  data_length_ = 0;
  free_length_ = length_;
}

int 
CircularBuffer::Write(const void* buffer, int length)
{
  //! buffer must be valid and length must > 0
  
  if (NULL == buffer || length <= 0)
    return -1;

  int write_length = (free_length_ > length ? length : free_length_);
  if (rpos_ > wpos_) {
    memcpy(buffer_ + wpos_, buffer, write_length);
    wpos_ += write_length;
  }
  else {
    int tail_len = length_ - wpos_;
    tail_len = (tail_len >= write_length ? write_length : tail_len);
    int left_len = write_length - tail_len;

    memcpy(buffer_ + wpos_, buffer, tail_len);
    wpos_ = (wpos_ + tail_len) % length_;
    if (left_len > 0) {
      memcpy(buffer_, (char*)buffer + tail_len, left_len);
      wpos_ += left_len;
    }
  }
  data_length_ += write_length;
  free_length_ -= write_length;

  return write_length;
}

int 
CircularBuffer::Read(int length, void* buffer)
{
  //! length must > 0 and buffer must be valid 

  if (length <= 0 || NULL == buffer)
    return -1;

  int read_length = (data_length_ > length ? length : data_length_);
  if (wpos_ > rpos_) {
    memcpy(buffer, buffer_ + rpos_, read_length);
    rpos_ += read_length;
  }
  else {
    int tail_len = length_ - rpos_;
    tail_len = (tail_len >= read_length ? read_length : tail_len);
    int left_len = read_length - tail_len;

    memcpy(buffer, buffer_ + rpos_, tail_len);
    rpos_ = (rpos_ + tail_len) % length_;
    if (left_len > 0) {
      memcpy((char*)buffer + tail_len, buffer_, left_len);
      rpos_ += left_len;
    }
  }
  data_length_ -= read_length;
  free_length_ += read_length;
  
  return read_length;
}

int 
CircularBuffer::Remove(int length)
{
  //! length must > 0
  if (length <= 0)
    return -1;

  int remove_length = (data_length_ > length ? length : data_length_);
  if (wpos_ > rpos_) {
    rpos_ += remove_length;
  }
  else {
    int tail_len = length_ - rpos_;
    tail_len = (tail_len >= remove_length ? remove_length : tail_len);

    rpos_ = (rpos_ + tail_len) % length_;
    rpos_ += (remove_length - tail_len);
  }
  data_length_ -= remove_length;
  free_length_ += remove_length;

  return remove_length;
}

}
