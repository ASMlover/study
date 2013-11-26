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
#include "thread.h"
#include "net.h"
#include "address.h"
#include "socket.h"
#include "logging.h"
#include "os_tool.h"
#if defined(_WINDOWS_) || defined(_MSC_VER)
# include "select_poll.h"
#elif defined(__linux__)
# include "epoll_poll.h"
#endif
#include "listener.h"




Listener::Listener(void)
  : running_(false)
  , listener_(NULL)
  , thread_(NULL)
  , poll_(NULL)
  , handler_(NULL)
{
}

Listener::~Listener(void)
{
  Stop();
}

bool 
Listener::Start(const char* ip, unsigned short port)
{
  if (NULL == poll_ || NULL == handler_)
    return false;

  listener_ = new Socket();
  if (NULL == listener_) {
    LOG_FAIL("new Socket failed\n");
    return false;
  }

  listener_->Open();
  listener_->SetNonBlock();
  listener_->SetReuseAddr();
  listener_->Bind(ip, port);
  listener_->Listen();

  thread_ = new Thread(&Listener::Routine, this);
  if (NULL == thread_) {
    LOG_FAIL("new Thread failed\n");
    return false;
  }

  running_ = true;
  thread_->Start();

  return true;
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
  if (NULL == self || NULL == self->poll_ || NULL == self->handler_)
    return;

  Socket s;
  Address addr;
  while (self->running_) {
    if (self->listener_->Accept(&s, &addr)) {
      self->poll_->Insert(s.fd(), kEventTypeRead | kEventTypeWrite);
      self->handler_->AcceptEvent(&s, &addr);
    }
    else {
      Tools::Sleep(1);
      continue;
    }

    s.Detach();
    addr.Detach();
  }
}
