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
#include "common.h"
#include "thread.h"
#include "conn_mgr.h"
#include "event_handler.h"
#include "worker.h"



Worker::Worker(void)
  : running_(false)
  , thread_(NULL)
  , conn_mgr_(NULL)
  , event_handler_(NULL)
{
  FD_ZERO(&rset_);
  FD_ZERO(&wset_);
}

Worker::~Worker(void)
{
}

void 
Worker::SetEventHandler(EventHandler* handler)
{
  event_handler_ = handler;
}

void 
Worker::Attach(ConnectorMgr* conn_mgr)
{
  conn_mgr_ = conn_mgr;
}

void 
Worker::Start(void)
{
  thread_ = new Thread(&Worker::Routine, this);
  if (NULL == thread_)
    LOG_FAIL("new Thread failed ...\n");

  running_ = true;
  thread_->Start();
}

void 
Worker::Stop(void)
{
  running_ = false;
  if (NULL != thread_) {
    thread_->Join();
    delete thread_;
    thread_ = NULL;
  }
}


void 
Worker::Routine(void* argument)
{
  Worker* self = static_cast<Worker*>(argument);
  if (NULL == self)
    return;

  while (self->running_) {
    self->conn_mgr_->InitSelectSets(&self->rset_, &self->wset_);

    int ret = select(0, &self->rset_, &self->wset_, NULL, NULL);
    if (SOCKET_ERROR == ret || 0 == ret) {
      Sleep(1);
      continue;
    }

    self->DispatchEvent(&self->rset_, EventHandler::kEventTypeRead);
    self->DispatchEvent(&self->wset_, EventHandler::kEventTypeWrite);
  }
}

void 
Worker::DispatchEvent(fd_set* set, int ev)
{
  for (unsigned int i = 0; i < set->fd_count; ++i) {
    Socket* s = conn_mgr_->GetConnector(set->fd_array[i]);

    if (NULL == s)
      continue;

    switch (ev) {
    case EventHandler::kEventTypeRead:
      event_handler_->ReadEvent(s);
      break;
    case EventHandler::kEventTypeWrite:
      event_handler_->WriteEvent(s);
      break;
    }
  }
}
