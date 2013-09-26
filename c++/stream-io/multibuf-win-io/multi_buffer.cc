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
#include "multi_buffer.h"




MultiBuffer::MultiBuffer(void)
  : mutex_()
  , file_handle_(INVALID_HANDLE_VALUE)
  , thread_(NULL)
  , start_event_(NULL)
  , quit_event_(NULL)
  , buffer_(NULL)
  , length_(0)
  , data_length_(0)
{
}

MultiBuffer::~MultiBuffer(void)
{
  Release();
}

void 
MultiBuffer::Create(const char* fname, int length)
{
  file_handle_ = CreateFileA(fname, 
      GENERIC_WRITE, FILE_SHARE_READ, NULL, 
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  assert(INVALID_HANDLE_VALUE != file_handle_);

  start_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
  assert(NULL != start_event_);

  quit_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
  assert(NULL != quit_event_);

  length_ = length;
  buffer_ = (char*)malloc(length_);
  assert(NULL != buffer_);

  thread_ = CreateThread(NULL, 0, &MultiBuffer::Routine, this, 0, NULL);
  if (NULL != thread_)
    WaitForSingleObject(start_event_, INFINITE);
  CloseHandle(start_event_);
}

void 
MultiBuffer::Release(void)
{
  if (NULL != quit_event_) {
    SetEvent(quit_event_);

    WaitForSingleObject(thread_, INFINITE);
    CloseHandle(thread_);
    CloseHandle(quit_event_);
    quit_event_ = NULL;
  }

  if (NULL != buffer_) {
    free(buffer_);
    buffer_ = NULL;
  }
  length_ = 0;
  data_length_ = 0;

  if (INVALID_HANDLE_VALUE != file_handle_) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
  }
}

int 
MultiBuffer::Write(const void* buffer, int length)
{
  MutexGuard lock(mutex_);
  memcpy(buffer_ + data_length_, buffer, length);
  data_length_ += length;

  return length;
}

DWORD WINAPI 
MultiBuffer::Routine(void* arg)
{
  MultiBuffer* self = static_cast<MultiBuffer*>(arg);
  assert(NULL != self);
  SetEvent(self->start_event_);

  while (true) {
    if (WAIT_OBJECT_0 == WaitForSingleObject(self->quit_event_, 5))
      break;

    MutexGuard lock(self->mutex_);
    if (self->data_length_ > (int)(self->length_ * 0.9)) {
      DWORD ret;
      SetFilePointer(self->file_handle_, 0, NULL, FILE_END);
      WriteFile(self->file_handle_, self->buffer_, 
          self->data_length_, &ret, NULL);
      self->data_length_ = 0;
    }
  }

  return 0;
}
