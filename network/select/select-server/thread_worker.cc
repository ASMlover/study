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
#include "select.h"
#include "event_handler.h"
#include "thread.h"
#include "thread_worker.h"



ThreadWorker::ThreadWorker(void)
  : thread_(NULL)
  , select_(NULL)
  , running_(false)
{
}

ThreadWorker::~ThreadWorker(void)
{
}

void 
ThreadWorker::Attach(Select* sel)
{
  select_ = sel;
}

void 
ThreadWorker::Start(void)
{
  Thread* thread_ = new Thread(&ThreadWorker::Routine, this);
  if (NULL == thread_)
    LOG_FAIL("new Thread failed ...\n");

  running_ = true;
  thread_->Start();
}

void 
ThreadWorker::Stop(void)
{
  running_ = false;
  thread_->Join();
  delete thread_;
}


void 
ThreadWorker::Routine(void* argument)
{
  ThreadWorker* self = static_cast<ThreadWorker*>(argument);
  if (NULL == self)
    return;

  while (self->running_) {
    self->select_->Poll();
  }
}
