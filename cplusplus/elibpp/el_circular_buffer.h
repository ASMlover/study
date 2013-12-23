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
#ifndef __EL_CIRCULAR_BUFFER_HEADER_H__
#define __EL_CIRCULAR_BUFFER_HEADER_H__

namespace el {

class CircularBuffer : private NonCopyable {
  enum { kDefBufferLength = 1024 * 1024 };
  char* buffer_;
  int length_;
  int rpos_;
  int wpos_;
  int data_length_;
  int free_length_;
public:
  explicit CircularBuffer(void);
  ~CircularBuffer(void);

  bool Create(int length = kDefBufferLength);
  void Release(void);
  void Clear(void);

  int Write(const void* buffer, int length);
  int Read(int length, void* buffer);
  int Remove(int length);
public:
  inline int length(void) const 
  {
    return length_;
  }

  inline int data_length(void) const 
  {
    return data_length_;
  }

  inline int free_length(void) const 
  {
    return free_length_;
  }
};

}

#endif  //! __EL_CIRCULAR_BUFFER_HEADER_H__
