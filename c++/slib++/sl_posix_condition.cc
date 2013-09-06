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
#include <errno.h>
#include "sl_condition.h"



#undef NANOSEC
#define NANOSEC   (1e9)


namespace sl {

static inline int 
cond_init(cond_t* cond)
{
  pthread_condattr_t attr;

  if (0 != pthread_condattr_init(&attr))
    return -1;
  if (0 != pthread_condattr_setlock(&attr, CLOCK_MONOTONIC))
    goto Exit2;

  if (0 != pthread_cond_init(cond, &attr))
    goto Exit2;
  if (0 != pthread_condattr_destroy(&attr))
    goto Exit;

  return 0;

Exit:
  pthread_cond_destroy(cond);
Exit2:
  pthread_condattr_destroy(&attr);
  return -1;
}

condition_t::condition_t(mutex_t& mutex)
  : mutex_(mutex)
{
  if (0 != cond_init(&cond_))
    abort();
}

condition_t::~condition_t(void)
{
  if (0 != pthread_cond_destroy(&cond_))
    abort();
}

void 
condition_t::signal(void)
{
  if (0 != pthread_cond_signal(&cond_))
    abort();
}

void 
condition_t::broadcast(void)
{
  if (0 != pthread_cond_broadcast(&cond_))
    abort();
}

void 
condition_t::wait(void)
{
  if (0 != pthread_cond_wait(&cond_, mutex_.get_mutex()))
    abort();
}

int  
condition_t::timedwait(unsigned int millitm)
{
  uint64_t timeout = (uint64_t)(millitm * 1e6);
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  timeout += (((uint64_t)ts.tv_sec) * NANOSEC + ts.tv_nsec);
  ts.tv_sec = timeout / NANOSEC;
  ts.tv_nsec = timeout % NANOSEC;
  int result = pthread_cond_timedwait(&cond_, mutex_.get_mutex(), &ts);

  if (0 == result)
    return 0;
  if (ETIMEDOUT == result)
    return -1;

  abort();
  return -1;
}

}
