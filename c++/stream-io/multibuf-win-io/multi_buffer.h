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
#ifndef __MULTI_BUFFER_HEADER_H__
#define __MULTI_BUFFER_HEADER_H__

#include "mutex.h"
#include "condition.h"

class MultiBuffer {
  enum {DEF_BUFLEN = 1024 * 16};

  Mutex mutex_;
  Condition cond_;
  HANDLE file_handle_;
  HANDLE thread_;
  HANDLE start_event_;
  HANDLE quit_event_;
  char* buffer_;
  int length_;
  int data_length_;

  MultiBuffer(const MultiBuffer&);
  MultiBuffer& operator =(const MultiBuffer&);
public:
  MultiBuffer(void);
  ~MultiBuffer(void);

  void Create(const char* fname, int length = DEF_BUFLEN);
  void Release(void);

  int Write(const void* buffer, int length);
private:
  static DWORD WINAPI Routine(void* arg);
};

#endif  //! __MULTI_BUFFER_HEADER_H__
