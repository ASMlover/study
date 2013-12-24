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
#ifndef __EL_CONNECTOR_HEADER_H__
#define __EL_CONNECTOR_HEADER_H__

namespace el {


class Connector : public Socket {
  uint32_t events_;
  SpinLock spinlock_;
  bool writable_;
  NetBuffer rbuf_;
  NetBuffer wbuf_;
public:
  explicit Connector(void);
  ~Connector(void);

  inline bool SetReadBuffer(uint32_t bytes) 
  {
    return rbuf_.Init(bytes);
  }

  inline bool SetWriteBuffer(uint32_t bytes)
  {
    return wbuf_.Init(bytes);
  }

  inline uint32_t events(void) const 
  {
    return events_;
  }

  inline void set_events(uint32_t events)
  {
    events_ = events;
  }
public:
  int Read(uint32_t bytes, char* buffer);
  int Write(const char* buffer, uint32_t bytes);

  bool WriteBufferEmpty(void);
  void SetWritable(bool writable = true);

  int DealWithAsyncRead(void);
  int DealWithAsyncWrite(void);
};


}

#endif  //! __EL_CONNECTOR_HEADER_H__
