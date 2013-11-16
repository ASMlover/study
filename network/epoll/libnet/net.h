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
#ifndef __NET_HEADER_H__
#define __NET_HEADER_H__

#include <sys/epoll.h>

enum NetType {
  kNetTypeInval = -1, 
  kNetTypeError = -1,
};

enum EventType {
  kEventTypeRead  = EPOLLIN, 
  kEventTypeWrite = EPOLLOUT, 
};



class NetLibrary {
  static bool loaded_;

  NetLibrary(const NetLibrary&);
  NetLibrary& operator =(const NetLibrary&);
public:
  explicit NetLibrary(void);
  ~NetLibrary(void);

  static NetLibrary& Singleton(void);
  bool Init(void);
  void Destroy(void);
};


class Socket;
class Address;
struct EventHandler {
  virtual ~EventHandler(void);
  virtual bool AcceptEvent(Socket* s, Address* addr);
  virtual void CloseEvent(Socket* s);
  virtual bool ReadEvent(Socket* s);
  virtual bool WriteEvent(Socket* s);
};


#endif  //! __NET_HEADER_H__
