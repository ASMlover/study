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
#include <assert.h>
#include "append_file.h"



AppendFile::AppendFile(void)
  : file_handle_(INVALID_HANDLE_VALUE)
  , buffer_(NULL)
  , heap_buf_(false)
  , buf_size_(0)
  , data_size_(0)
{
}

AppendFile::~AppendFile(void)
{
  Close();
}

void 
AppendFile::SetBuffer(char* buffer, size_t size)
{
  if (NULL == buffer || 0 == size)
    return;

  if (NULL != buffer) {
    if (data_size_ > 0) {
      DWORD ret;
      SetFilePointer(file_handle_, 0, NULL, FILE_END);
      WriteFile(file_handle_, buffer_, data_size_, &ret, NULL);
      data_size_ = 0;
    }

    if (heap_buf_) {
      free(buffer_);
      heap_buf_ = false;
    }
    buffer_ = NULL;
  }

  data_size_ = 0;
  buf_size_ = size;
  if (NULL == buffer) {
    buffer_ = (char*)malloc(buf_size_);
    assert(NULL != buffer_);
    heap_buf_ = true;
  }
  else {
    buffer_ = buffer;
    heap_buf_ = false;
  }
}

bool 
AppendFile::Open(const char* fname)
{
  file_handle_ = CreateFileA(fname, 
      GENERIC_READ | GENERIC_WRITE, 
      FILE_SHARE_READ | FILE_SHARE_WRITE, 
      NULL, 
      OPEN_ALWAYS, 
      FILE_ATTRIBUTE_NORMAL, 
      NULL);
  if (INVALID_HANDLE_VALUE == file_handle_)
    return false;

  buf_size_ = DEF_BUFSIZE;
  buffer_ = (char*)malloc(buf_size_);
  if (NULL == buffer_) 
    goto Exit;
  heap_buf_ = true;
  data_size_ = 0;

  return true;

Exit:
  CloseHandle(file_handle_);
  return false;
}

void 
AppendFile::Close(void)
{
  if (NULL != buffer_) {
    if (INVALID_HANDLE_VALUE != file_handle_ && data_size_ > 0) {
      DWORD ret;
      SetFilePointer(file_handle_, 0, NULL, FILE_END);
      WriteFile(file_handle_, buffer_, data_size_, &ret, NULL);
      data_size_ = 0;
    }

    if (heap_buf_) {
      free(buffer_);
      heap_buf_ = false;
    }
  }
  buf_size_ = 0;
  data_size_ = 0;

  if (INVALID_HANDLE_VALUE != file_handle_) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
  }
}

size_t 
AppendFile::Write(const void* buffer, size_t size)
{
  if (NULL == buffer || 0 == size)
    return 0;

  DWORD ret;
  if (buf_size_ - data_size_ < size || buf_size_ < size) {
    if (data_size_ > 0) {
      SetFilePointer(file_handle_, 0, NULL, FILE_END);
      WriteFile(file_handle_, buffer_, data_size_, &ret, NULL);
      data_size_ = 0;
    }
  }

  if (buf_size_ < size) {
    SetFilePointer(file_handle_, 0, NULL, FILE_END);
    WriteFile(file_handle_, buffer, size, &ret, NULL);
  }
  else {
    memcpy(buffer_ + data_size_, buffer, size);
    data_size_ += size;
  }

  return size;
}
