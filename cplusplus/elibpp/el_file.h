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
#ifndef __EL_FILE_HEADER_H__
#define __EL_FILE_HEADER_H__

namespace el {

class File : private NonCopyable {
  enum { kDefBufferSize = 16 * 1024 };
#if defined(PLATFORM_WIN)
  HANDLE fd_;
#elif defined(PLATFORM_LINUX)
  int fd_;
#endif
  char*   buffer_;
  bool    allocated_;
  size_t  buf_size_;
  size_t  data_size_;
public:
  explicit File(void);
  ~File(void);

  void SetBuffer(char* buffer, size_t size);
  bool Open(const char* fname, bool append = false);
  void Close(void);

  //! be sure that length of buffer is less than data_size_
  size_t Write(const void* buffer, size_t size);
};

}

#endif  //! __EL_FILE_HEADER_H__
