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
#include <errno.h>
#include <stdlib.h>
#include "mutex.h"
#include "condition.h"




static inline void 
pthread_call(const char* label, int result)
{
  if (0 != result) {
    fprintf(stderr, "pthread %s: %s\n", label, strerror(result));
    abort();
  }
}




condition_t::condition_t(mutex_t& mutex)
  : mutex_(mutex)
{
  pthread_call("init condition", pthread_cond_init(&cond_, NULL));
}

condition_t::~condition_t(void)
{
  pthread_call("destroy condition", pthread_cond_destroy(&cond_));
}

void 
condition_t::signal(void)
{
  pthread_call("signal", pthread_cond_signal(&cond_));
}

void 
condition_t::siganl_all(void)
{
  pthread_call("broadcast", pthread_cond_broadcast(&cond_));
}

void 
condition_t::wait(void)
{
  pthread_call("wait", pthread_cond_wait(&cond_, mutex.get_mutex()));
}
