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
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "append_file.h"





#define UNUSED(x) { (x) = (x); }



AppendFile::AppendFile(size_t buf_size)
  : filelock_()
  , fd_(-1)
  , buffer_(NULL)
  , buf_size_(buf_size)
  , data_size_(0)
{
}

AppendFile::~AppendFile(void)
{
  Close();
}

bool 
AppendFile::Open(const char* fname)
{
  fd_ = open(fname, O_CREAT | O_RDWR | O_APPEND, 0666);
  if (-1 == fd_)
    return false;

  if (buf_size_ < DEF_BUFSIZE)
    buf_size_ = DEF_BUFSIZE;

  buffer_ = (char*)malloc(buf_size_);
  if (NULL == buffer_)
    goto Exit;

  return true;

Exit:
  close(fd_);
  fd_ = -1;
  return false;
}

void 
AppendFile::Close(void)
{
  if (NULL != buffer_) {
    if (data_size_ > 0 && -1 != fd_) {
      ssize_t ret = write(fd_, buffer_, data_size_);
      UNUSED(ret);
    }
    data_size_ = 0;

    free(buffer_);
    buffer_ = NULL;
  }

  if (-1 != fd_) {
    close(fd_);
    fd_ = -1;
  }
}

size_t 
AppendFile::Write(const void* buffer, size_t size)
{
  size_t ret;
  {
    FileLockGuard lock(filelock_);
    ret = WriteUnlocked(buffer, size);
  }

  return ret;
}

size_t 
AppendFile::WriteUnlocked(const void* buffer, size_t size)
{
  if (NULL == buffer || 0 == size)
    return 0;

  if (buf_size_ - data_size_ < size) {
    if (data_size_ > 0) {
      int ret = write(fd_, buffer_, data_size_);
      UNUSED(ret);
      data_size_ = 0;
    }
  }

  if (buf_size_ < size) {
    int ret = write(fd_, buffer_, data_size_);
    UNUSED(ret);
  }
  else {
    memcpy(buffer_ + data_size_, buffer, size);
    data_size_ += size;
  }

  return size;
}
