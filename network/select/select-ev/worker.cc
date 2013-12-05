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
#include "select_poll.h"
#include "connector_mgr.h"
#include "worker.h"


Worker::Worker(void)
  : running_(false)
  , worker_id_(-1)
  , poll_(NULL)
  , thread_(NULL)
  , conn_mgr_(NULL)
  , dispatcher_(NULL)
{
}

Worker::~Worker(void)
{
  Stop();
}


bool 
Worker::Start(void)
{
  if (-1 == worker_id_ || NULL == conn_mgr_ || NULL == dispatcher_)
    return false;

  poll_ = new SelectPoll();
  if (NULL == poll_)
    return false;

  do {
    thread_ = new Thread();
    if (NULL == thread_)
      break;

    running_ = true;
    thread_->Start(&Worker::Routine, this);

    return true;
  } while (0);

  Stop();
  return false;
}

void 
Worker::Stop(void)
{
  running_ = false;

  if (NULL != thread_) {
    thread_->Stop();

    delete thread_;
    thread_ = NULL;
  }

  if (NULL != poll_) {
    delete poll_;
    poll_ = NULL;
  }
}

bool 
Worker::AddConnector(int fd, Connector* conn)
{
  if (NULL == poll_ || NULL == conn)
    return false;

  return poll_->Insert(fd, conn);
}

void 
Worker::Routine(void* argument)
{
  Worker* self = static_cast<Worker*>(argument);
  if (NULL == self || NULL == self->poll_ || NULL == self->dispatcher_)
    return;

  while (self->running_) {
    if (!self->poll_->Dispatch(self->dispatcher_, 10)) {
      Sleep(10);
      continue;
    }
  }
}
