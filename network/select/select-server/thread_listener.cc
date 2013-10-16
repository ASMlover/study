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
#include <windows.h>
#include "global.h"
#include "socket.h"
#include "thread.h"
#include "select.h"
#include "event_handler.h"
#include "thread_listener.h"



ThreadListener::ThreadListener(void)
  : listener_(NULL)
  , thread_(NULL)
  , running_(false)
  , select_(NULL)
  , getHandler_(NULL)
{
}

ThreadListener::~ThreadListener(void)
{
}

void 
ThreadListener::Attach(Select* sel, EventHandler* (*getHandler)(Socket*))
{
  select_ = sel;
  getHandler_ = getHandler;
}

void 
ThreadListener::Start(const char* ip, unsigned short port)
{
  listener_ = new Socket();
  if (NULL == listener_)
    LOG_FAIL("new Socket failed ...\n");
  listener_->Open();
  listener_->Bind(ip, port);
  listener_->Listen();

  running_ = true;
  thread_ = new Thread(&ThreadListener::Routine, this);
  if (NULL == thread_)
    LOG_FAIL("new Thread failed ...\n");
  thread_->Start();
}

void 
ThreadListener::Stop(void)
{
  running_ = false;

  thread_->Join();
  delete thread_;
  listener_->Close();
  delete listener_;
}



void 
ThreadListener::Routine(void* argument)
{
  ThreadListener* self = static_cast<ThreadListener*>(argument);
  if (NULL == self)
    return;

  fprintf(stdout, "ThreadListener working ...\n");
  while (self->running_) {
    Socket* s = new Socket();
    if (NULL == s) {
      LOG_ERR("new Socket error ...\n");
      continue;
    }
    self->listener_->Accept(s, NULL);

    EventHandler* eh = self->getHandler_(s);
    if (NULL == eh) {
      LOG_ERR("eh is NULL ...\n");
      s->Close();
      delete s;
      Sleep(1);
      continue;
    }

    self->select_->Insert(eh, 
        EventHandler::ET_READ | EventHandler::ET_WRITE);
  }
}
