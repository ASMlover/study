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
#include "select_ev.h"
#include "thread.h"
#include "address.h"
#include "socket.h"
#include "connector_mgr.h"
#include "listener.h"



Listener::Listener(void)
  : running_(false)
  , listener_(NULL)
  , thread_(NULL)
  , conn_mgr_(NULL)
{
}

Listener::~Listener(void)
{
  Stop();
}

bool 
Listener::Start(const char* ip, unsigned short port)
{
  if (NULL == conn_mgr_)
    return false;

  listener_ = new Socket();
  if (NULL == listener_)
    return false;
  listener_->Open();
  listener_->SetReuseAddr();
  listener_->Bind(ip, port);
  listener_->Listen();

  do {
    thread_ = new Thread();
    if (NULL == thread_)
      break;

    running_ = true;
    thread_->Start(&Listener::Routine, this);

    return true;
  } while (0);

  Stop();
  return false;
}

void 
Listener::Stop(void)
{
  running_ = false;

  if (NULL != thread_) {
    thread_->Stop();

    delete thread_;
    thread_ = NULL;
  }
  
  if (NULL != listener_) {
    listener_->Close();

    delete listener_;
    listener_ = NULL;
  }
}


void 
Listener::Routine(void* argument)
{
  Listener* self = static_cast<Listener*>(argument);
  if (NULL == self || NULL == self->conn_mgr_)
    return;

  Socket s;
  Address addr;
  while (self->running_) {
    if (!self->listener_->Accept(&s, &addr)) {
      Sleep(1);
      continue;
    }
    else {
      int fd = s.fd();
      int worker_id = self->conn_mgr_->SuitableWorker();
      Connector* conn = self->conn_mgr_->Insert(fd, worker_id);
      if (NULL != conn) {
        //! TODO:
        //! add event 
      }
      else {
        s.Close();
      }

      s.Detach();
      addr.Detach();
    }
  }
}
