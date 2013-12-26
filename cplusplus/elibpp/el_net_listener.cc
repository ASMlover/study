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
#include "el_net_internal.h"
#include "el_time.h"
#include "el_address.h"
#include "el_socket.h"
#include "el_net_worker.h"
#include "el_network_handler.h"
#include "el_net_listener.h"



namespace el {


NetListener::NetListener(void)
  : running_(false)
  , listener_(NULL)
  , thread_(NULL)
  , network_(NULL)
  , conn_holder_(NULL)
{
}

NetListener::~NetListener(void)
{
  Stop();
}


bool 
NetListener::Start(const char* ip, uint16_t port)
{
  if (NULL == network_ || NULL == conn_holder_)
    return false;

  if (NULL == (listener_ = new Socket()))
    return false;

  do {
    if (!listener_->Open() 
        || !listener_->SetNonBlock() 
        || !listener_->SetReuseAddr() 
        || !listener_->Bind(ip, port) 
        || !listener_->Listen())
      break;

    if (NULL == (thread_ = new Thread(&NetListener::Routine, this)))
      break;

    running_ = true;
    thread_->Start();

    return true;
  } while (0);

  Stop();
  return false;
}

void 
NetListener::Stop(void)
{
  running_ = false;

  if (NULL != thread_) {
    thread_->Join();

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
NetListener::Routine(void* argument)
{
  NetListener* self = static_cast<NetListener*>(argument);
  if (NULL == self || NULL == self->listener_ 
      || NULL == self->network_ || NULL == self->conn_holder_)
    return;

  Socket s;
  Address addr;
  while (self->running_) {
    if (!self->listener_->Accept(&s, &addr)) {
      el::Sleep(1);
      continue;
    }
    else {
      Connector* conn = self->conn_holder_->Insert(s.fd());
      if (NULL != conn) {
        self->network_->SuitableWorker().AddConnector(conn);
        self->network_->MarkNextSuitableWorker();
      }
      else {
        s.Close();
      }

      s.Detach();
      addr.Detach();
    }
  }
}


}
