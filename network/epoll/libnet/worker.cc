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
#include <stdlib.h>
#include "net.h"
#include "logging.h"
#include "thread.h"
#if defined(_WINDOWS_) || defined(_MSC_VER)
# include "select_poll.h"
#elif defined(__linux__)
# include "epoll_poll.h"
#endif
#include "worker.h"



Worker::Worker(void)
  : running_(false)
  , rthread_(NULL)
  , wthread_(NULL)
  , poll_(NULL)
{
}

Worker::~Worker(void)
{
  Stop();
}

bool 
Worker::Start(void)
{
  if (NULL == poll_)
    return false;

  rthread_ = new Thread(&Worker::ReadRoutine, this);
  if (NULL == rthread_) {
    LOG_FAIL("new Thread failed\n");
    return false;
  }

  wthread_ = new Thread(&Worker::WriteRoutine, this);
  if (NULL == wthread_) {
    LOG_FAIL("new Thread failed\n");
    return false;
  }

  running_ = true;
  rthread_->Start();
  wthread_->Start();

  return true;
}

void 
Worker::Stop(void)
{
  running_ = false;

  if (NULL != rthread_) {
    rthread_->Join();
    delete rthread_;

    rthread_ = NULL;
  }

  if (NULL != wthread_) {
    wthread_->Join();
    delete wthread_;

    wthread_ = NULL;
  }
}


void 
Worker::ReadRoutine(void* argument)
{
  Worker* self = static_cast<Worker*>(argument);
  if (NULL == self)
    return;

  while (self->running_) {
    if (!self->poll_->Polling(kEventTypeRead)) {
      Sleep(1);
      continue;
    }
  }
}

void 
Worker::WriteRoutine(void* argument)
{
  Worker* self = static_cast<Worker*>(argument);
  if (NULL == self)
    return;

  while (self->running_) {
    if (!self->poll_->Polling(kEventTypeWrite)) {
      Sleep(1);
      continue;
    }
  }
}
