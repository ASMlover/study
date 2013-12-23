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
#ifndef __EL_NET_BUFFER_HEADER_H__
#define __EL_NET_BUFFER_HEADER_H__


namespace el {


class NetBuffer : private NonCopyable {
  char* buffer_;
  int   length_;
  int   storage_;
public:
  enum { kDefaultStorage = 16 * 1024 };
  explicit NetBuffer(void);
  ~NetBuffer(void);

  inline const char* buffer(void) const 
  {
    return buffer_;
  }

  inline int length(void) const 
  {
    return length_;
  }

  inline char* free_buffer(void) const 
  {
    return buffer_ + length_;
  }

  inline int free_length(void) const 
  {
    return storage_ - length_;
  }
public:
  bool Init(int storage = kDefaultStorage);
  void Destroy(void);

  int Put(const char* buffer, int bytes);
  int Get(int bytes, char* buffer);
  int Increment(int bytes);
  int Decrement(int bytes);

  bool Regrow(void);
};


}

#endif  //! __EL_NET_BUFFER_HEADER_H__
