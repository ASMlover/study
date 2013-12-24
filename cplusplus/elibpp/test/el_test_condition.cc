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
#include "el_test_header.h"
#include "../el_condition.h"



struct WorkerConfig {
  el::Mutex mutex;
  el::Condition cond;
  bool use_signal_all;
  volatile bool posted;

  WorkerConfig(void)
    : mutex()
    , cond(mutex)
    , use_signal_all(false)
    , posted(false)
  {
  }
};



static inline void 
Worker(void* arg)
{
  WorkerConfig* wc = static_cast<WorkerConfig*>(arg);
  el_sleep(100);

  wc->mutex.Lock();
  UNIT_ASSERT(!wc->posted);
  if (wc->use_signal_all)
    wc->cond.SignalAll();
  else 
    wc->cond.Signal();
  wc->posted = true;
  wc->mutex.Unlock();
}

UNIT_IMPL(ConditionSignal)
{
  WorkerConfig wc;

  el::Thread thread(Worker, (void*)&wc);
  thread.Start();

  wc.mutex.Lock();
  wc.cond.Wait();
  UNIT_ASSERT(wc.posted);
  wc.mutex.Unlock();

  thread.Join();
}

UNIT_IMPL(ConditionSignalAll)
{
  WorkerConfig wc;
  wc.use_signal_all = true;

  el::Thread thread(Worker, (void*)&wc);
  thread.Start();

  wc.mutex.Lock();
  wc.cond.Wait();
  UNIT_ASSERT(wc.posted);
  wc.mutex.Unlock();

  thread.Join();
}
