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
#include "net.h"
#include "logging.h"
#include "thread.h"
#include "socket.h"
#include "select_poll.h"
#include "select_listener.h"




SelectListener::SelectListener(void)
  : running_(false)
  , listener_(NULL)
  , thread_(NULL)
  , poll_(NULL)
  , handler_(NULL)
{
}

SelectListener::~SelectListener(void)
{
  Stop();
}

bool 
SelectListener::Start(const char* ip, unsigned short port)
{
  if (NULL == poll_ || NULL == handler_)
    return false;

  listener_ = new Socket();
  if (NULL == listener_) {
    LOG_FAILX("new Socket failed\n");
    return false;
  }
  listener_->Open();
  listener_->Bind(ip, port);
  listener_->Listen();

  thread_ = new Thread(&SelectListener::Routine, this);
  if (NULL == thread_) {
    LOG_FAILX("new Thread failed\n");
    return false;
  }

  running_ = true;
  thread_->Start();

  return true;
}

void 
SelectListener::Stop(void)
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
SelectListener::Routine(void* argument)
{
  SelectListener* self = static_cast<SelectListener*>(argument);
  if (NULL == self)
    return;

  Socket s;
  struct sockaddr addr;
  while (self->running_) {
    if (self->listener_->Accept(&s, &addr)) {
      self->poll_->Insert(s.fd(), EventHandler::kEventTypeRead);
      self->handler_->AcceptEvent(s.fd(), &addr);
    }
    else {
      Sleep(1);
      continue;
    }

    s.Detach();
  }
}
