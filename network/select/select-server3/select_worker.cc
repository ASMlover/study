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
#include "logging.h"
#include "thread.h"
#include "select_poll.h"
#include "select_worker.h"



SelectWorker::SelectWorker(void)
  : running_(false)
  , thread_(NULL)
  , poll_(NULL)
{
}

SelectWorker::~SelectWorker(void)
{
  Stop();
}

bool 
SelectWorker::Start(void)
{
  if (NULL == poll_)
    return false;

  thread_ = new Thread(&SelectWorker::Routine, this);
  if (NULL == thread_) {
    LOG_FAIL("new Thread failed\n");
    return false;
  }

  running_ = true;
  thread_->Start();

  return true;
}

void 
SelectWorker::Stop(void)
{
  running_ = false;

  if (NULL != thread_) {
    thread_->Join();
    delete thread_;
    thread_ = NULL;
  }
}


void 
SelectWorker::Routine(void* argument)
{
  SelectWorker* self = static_cast<SelectWorker*>(argument);
  if (NULL == self)
    return;

  while (self->running_) {
    if (!self->poll_->Polling()) {
      Sleep(1);
      continue;
    }
  }
}
