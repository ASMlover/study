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
#ifndef _WINDOWS_ 
# include <winsock2.h>
#endif
#include "common.h"
#include "thread.h"
#include "socket.h"
#include "conn_mgr.h"
#include "listener.h"


Listener::Listener(void)
  : conn_mgr_(NULL)
  , thread_(NULL)
  , socket_(NULL)
  , running_(false)
{
}

Listener::~Listener(void)
{
  Stop();
}

void 
Listener::Attach(ConnectorMgr* conn_mgr)
{
  conn_mgr_ = conn_mgr;
}

void 
Listener::Start(const char* ip, unsigned short port)
{
  thread_ = new Thread(&Listener::Routine, this);
  if (NULL == thread_)
    LOG_FAIL("create Listener thread failed ...\n");
  
  socket_ = new Socket();
  if (NULL == socket_)
    LOG_FAIL("new Socket failed ...\n");
  socket_->Open();
  socket_->Bind(ip, port);
  socket_->Listen();

  running_ = true;
  thread_->Start();
}

void 
Listener::Stop(void)
{
  running_ = false;
  if (NULL != thread_) {
    thread_->Join();
    delete thread_;
    thread_ = NULL;
  }
  if (NULL != socket_) {
    socket_->Close();
    delete socket_;
    socket_ = NULL;
  }
}



void 
Listener::Routine(void* arg)
{
  Listener* self = static_cast<Listener*>(arg);
  if (NULL == self)
    return;

  Socket s;
  while (self->running_) {
    if (self->socket_->Accept(&s, NULL)) {
      self->conn_mgr_->Insert(s.fd());
    }
    else {
      Sleep(1);
      continue;
    }

    s.Detach();
  }
}
