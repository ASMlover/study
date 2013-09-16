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
#ifndef __EL_POSIX_MUTEX_HEADER_H__
#define __EL_POSIX_MUTEX_HEADER_H__

#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



static inline void 
pthreadCall(const char* label, int result)
{
  if (0 != result) {
    fprintf(stderr, "%s : %s\n", strerror(result));
    abort();
  }
}


namespace sl {

class Mutex {
  pthread_mutex_t mutex_;

  Mutex(const Mutex&);
  Mutex& operator =(const Mutex&);
public:
  Mutex(void)
  {
    pthreadCall("Mutex init", pthread_mutex_init(&mutex_, NULL));
  }

  ~Mutex(void)
  {
    pthreadCall("Mutex destroy", pthread_mutex_destroy(&mutex_));
  }

  void lock(void)
  {
    pthreadCall("Mutex lock", pthread_mutex_lock(&mutex_));
  }

  void unlock(void)
  {
    pthreadCall("Mutex unlock", pthread_mutex_unlock(&mutex_));
  }

  pthread_mutex_t* getMutex(void)
  {
    return  mutex_;
  }
};

}

#endif  //! __EL_POSIX_MUTEX_HEADER_H__
