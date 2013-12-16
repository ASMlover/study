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
#ifndef __CONNECTOR_DISPATCHER_HEADER_H__
#define __CONNECTOR_DISPATCHER_HEADER_H__

#include <map>
#include "locker.h"

class Connector;
class ConnectorDispatcher : public Dispatcher {
  enum {kDefaultBufferLen = 16 * 1024};
  uint32_t rbytes_;
  uint32_t wbytes_;
  SpinLock spinlock_;
  EventHandler* handler_;
  std::map<int, Connector*> connectors_;

  ConnectorDispatcher(const ConnectorDispatcher&);
  ConnectorDispatcher& operator =(const ConnectorDispatcher&);
public:
  explicit ConnectorDispatcher(void);
  ~ConnectorDispatcher(void);

  inline void Attach(EventHandler* handler) 
  {
    handler_ = handler;
  }

  inline void SetBuffer(
      uint32_t rbytes = kDefaultBufferLen, 
      uint32_t wbytes = kDefaultBufferLen)
  {
    rbytes_ = rbytes;
    wbytes_ = wbytes;
  }
public:
  void CloseAll(void);
  Connector* Insert(int fd);
  void Remove(int fd);
public:
  virtual bool DispatchReader(Poller* poller, Connector* conn);
  virtual bool DispatchWriter(Poller* poller, Connector* conn);
};

#endif  //! __CONNECTOR_DISPATCHER_HEADER_H__
