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
#include "../elib_internal.h"
#include "../el_file.h"


#if defined(UNUSED)
# undef UNUSED
#endif
#define UNUSED(x) {(x) = (x);}


namespace el {

File::File(void)
  : fd_(-1)
  , buffer_(NULL)
  , allocated_(false)
  , buf_size_(0)
  , data_size_(0)
{
}

File::~File(void)
{
  Close();
}

void 
File::SetBuffer(char* buffer, size_t size)
{
  //! this function must be used before any Write 
  if (NULL != buffer_ && allocated_)
    free(buffer_);

  data_size_ = 0;
  buf_size_ = size;
  if (NULL == buffer) {
    buffer_ = (char*)malloc(buf_size_);
    allocated_ = true;
  }
  else {
    buffer_ = buffer;
    allocated_ = false;
  }
}

bool 
File::Open(const char* fname, bool append)
{
  int oflags = O_RDWR | O_CREAT;
  if (append)
    oflags |= O_APPEND;

  fd_ = open(fname, oflags, 0666);
  if (-1 == fd_)
    return false;

  buf_size_ = kDefBufferSize;
  buffer_ = (char*)malloc(buf_size_);
  if (NULL == buffer_)
    goto Exit;
  allocated_ = true;
  data_size_ = 0;

  return true;

Exit:
  close(fd_);
  fd_ = -1;
  return false;
}

void 
File::Close(void)
{
  if (NULL != buffer_) {
    if (data_size_ > 0 && -1 != fd_) {
      ssize_t ret = write(fd_, buffer_, data_size_);
      UNUSED(ret)
    }

    if (allocated_) {
      free(buffer_);
      allocated_ = false;
    }
    buffer_ = NULL;
  }
  data_size_ = 0;

  if (-1 != fd_) {
    close(fd_);
    fd_ = -1;
  }
}

size_t 
File::Write(const void* buffer, size_t size)
{
  if (NULL == buffer || 0 == size)
    return 0;

  size_t free_size = buf_size_ - data_size_;
  if (free_size > size) {
    memcpy(buffer_ + data_size_, buffer, size);
    data_size_ += size;
  }
  else {
    size_t copy_size = size - free_size;
    memcpy(buffer_ + data_size_, buffer, free_size);
    ssize_t ret = write(fd_, buffer_, kDefBufferSize);
    UNUSED(ret)
    data_size_ = 0;

    if (copy_size > 0) {
      memcpy(buffer_ + data_size_, (char*)buffer + free_size, copy_size);
      data_size_ += copy_size;
    }
  }

  return size;
}

}
