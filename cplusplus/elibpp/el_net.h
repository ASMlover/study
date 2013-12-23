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
#ifndef __EL_NET_HEADER_H__
#define __EL_NET_HEADER_H__

namespace el {


enum NetType {
  kNetTypeInval = -1, 
  kNetTypeError = -1, 
};

enum EventType {
  kEventTypeUnknown = 0x00, 
  kEventTypeRead    = 0x01, 
  kEventTypeWrite   = 0x02,
};


class NetLibrary : public Singleton<NetLibrary>, private NonCopyable {
  static bool loaded_;
public:
  explicit NetLibrary(void);
  ~NetLibrary(void);

  bool Init(void);
  void Destroy(void);
};


class Connector;
struct EventHandler {
  virtual ~EventHandler(void) {}
  virtual bool AcceptEvent(Connector* conn) {return true;}
  virtual void CloseEvent(Connector* conn) {}
  virtual bool ReadEvent(Connector* conn) {return true;}
};

}

#endif  //! __EL_NET_HEADER_H__
