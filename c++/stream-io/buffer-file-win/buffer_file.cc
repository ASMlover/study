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
#include "buffer_file.h"




BufferFile::BufferFile(void)
  : file_handle_(INVALID_HANDLE_VALUE)
  , length_(0)
{
}

BufferFile::~BufferFile(void)
{
  Close();
}

bool 
BufferFile::Open(const char* filename)
{
  if (NULL == filename)
    return false;

  file_handle_ = CreateFileA(filename, 
      GENERIC_WRITE, FILE_SHARE_READ, NULL, 
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  return (INVALID_HANDLE_VALUE != file_handle_);
}

void 
BufferFile::Close(void)
{
  if (INVALID_HANDLE_VALUE != file_handle_ && length_ > 0) {
    DWORD ret;
    SetFilePointer(file_handle_, 0, NULL, FILE_END);
    WriteFile(file_handle_, buffer_, length_, &ret, NULL);
  }

  if (INVALID_HANDLE_VALUE != file_handle_) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
  }
  length_ = 0;
}

int 
BufferFile::Write(const void* buffer, int length)
{
  if (NULL == buffer || length <= 0)
    return -1;

  DWORD ret;
  if (length > BUFSIZE) {
    if (length_ > 0) {
      SetFilePointer(file_handle_, 0, NULL, FILE_END);
      WriteFile(file_handle_, buffer_, length_, &ret, NULL);
      length_ = 0;
    }
    SetFilePointer(file_handle_, 0, NULL, FILE_END);
    WriteFile(file_handle_, buffer, length, &ret, NULL);
  }
  else {
    if (BUFSIZE - length_ < length) {
      SetFilePointer(file_handle_, 0, NULL, FILE_END);
      WriteFile(file_handle_, buffer_, length_, &ret, NULL);
      length_ = 0;
    }
    
    memcpy(buffer_ + length_, buffer, length);
    length_ += length;
  }
  return length;
}
