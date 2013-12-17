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
#include "libnet_internal.h"
#if defined(PLATFORM_WIN)
# include "select.h"
  typedef Select  EventPoller;
#elif defined(PLATFORM_POSIX)
# include "epoll.h"
  typedef Epoll   EventPoller;
#endif
#include "tools.h"
#include "thread.h"
#include "worker.h"



Worker::Worker(void)
  : running_(false)
  , poller_(NULL)
  , thread_(NULL)
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
  if (NULL == dispatcher_)
    return false;

  poller_ = new EventPoller();
  if (NULL == poller_)
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
  return true;
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

  if (NULL == poller_) {
    delete poller_;
    poller_ = NULL;
  }
}

bool 
Worker::AddConnector(Connector* conn)
{
  if (NULL == poller_)
    return false;

  return poller_->Insert(conn);
}



void 
Worker::Routine(void* argument)
{
  Worker* self = static_cast<Worker*>(argument);
  if (NULL == self || NULL == self->dispatcher_ || NULL == self->poller_)
    return;

  while (self->running_) {
    if (!self->poller_->Dispatch(self->dispatcher_, 10)) {
      tools::Sleep(1);
      continue;
    }
  }
}
