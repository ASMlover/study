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
#include "sl_test_header.h"
#include "sl_mutex.h"
#include "sl_condition.h"
#include "sl_thread.h"


struct worker_config {
  sl::mutex_t mutex;
  sl::condition_t cond;
  bool use_broadcast;
  volatile bool posted;

  worker_config(void)
    : mutex()
    , cond(mutex)
    , use_broadcast(false)
    , posted(false)
  {
  }
};




static inline void 
worker(void* arg)
{
  worker_config* wc = static_cast<worker_config*>(arg);

#if defined(_WINDOWS_) || defined(_MSC_VER)
  Sleep(100);
#elif defined(_MSC_VER)
  usleep(100 * 1000);
#endif

  wc->mutex.lock();
  fprintf(stdout, "\t\tenter worker lock ...\n");
  ASSERT(!wc->posted);
  wc->posted = true;
  if (wc->use_broadcast)
    wc->cond.broadcast();
  else 
    wc->cond.signal();
  wc->mutex.unlock();
  fprintf(stdout, "\t\tleave worker lock ...\n");
}


TEST_IMPL(condition_1)
{
  worker_config wc;

  sl::thread_t thread(worker, (void*)&wc);
  thread.start();

  wc.mutex.lock();
  wc.cond.wait();
  ASSERT(wc.posted);
  wc.mutex.unlock();

  thread.join();
}

TEST_IMPL(condition_2)
{
  worker_config wc;
  wc.use_broadcast = true;

  sl::thread_t thread(worker, (void*)&wc);
  thread.start();

  wc.mutex.lock();
  wc.cond.wait();
  ASSERT(wc.posted);
  wc.mutex.unlock();

  thread.join();
}
