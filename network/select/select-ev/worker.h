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
#ifndef __WORKER_HEADER_H__
#define __WORKER_HEADER_H__

struct EventPoll;
class EventDispatcher;
class Thread;
class Connector;
class ConnectorMgr;
class Worker {
  bool running_;
  int worker_id_;
  EventPoll* poll_;
  Thread* thread_;
  ConnectorMgr* conn_mgr_;
  EventDispatcher* dispatcher_;

  Worker(const Worker&);
  Worker& operator =(const Worker&);
public:
  explicit Worker(void);
  ~Worker(void);

  inline int worker_id(void) const 
  {
    return worker_id_;
  }

  inline void Attach(int worker_id) 
  {
    worker_id_ = worker_id;
  }

  inline void Attach(ConnectorMgr* conn_mgr)
  {
    conn_mgr_ = conn_mgr;
  }

  inline void Attach(EventDispatcher* dispatcher)
  {
    dispatcher_ = dispatcher;
  }
public:
  bool Start(void);
  void Stop(void);

  bool AddConnector(int fd, Connector* conn);
private:
  static void Routine(void* argument);
};

#endif  //! __WORKER_HEADER_H__
