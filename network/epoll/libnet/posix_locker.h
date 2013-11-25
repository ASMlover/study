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
#ifndef __POSIX_LOCKER_HEADER_H__
#define __POSIX_LOCKER_HEADER_H__

#include <pthread.h>
#include "logging.h"

class Mutex {
  pthread_mutex_t mutex_;

  Mutex(const Mutex&);
  Mutex& operator =(const Mutex&);
public:
  explicit Mutex(void)
  {
    if (0 != pthread_mutex_init(&mutex_, NULL))
      LOG_FAIL("pthread_mutex_init failed\n");
  }

  ~Mutex(void)
  {
    if (0 != pthread_mutex_destroy(&mutex_))
      LOG_FAIL("pthread_mutex_destroy failed\n");
  }

  inline void Lock(void)
  {
    if (0 != pthread_mutex_lock(&mutex_))
      LOG_FAIL("pthread_mutex_lock faield\n");
  }

  inline void Unlock(void)
  {
    if (0 != pthread_mutex_unlock(&mutex_))
      LOG_FAIL("pthread_mutex_unlock failed\n");
  }
};


class SpinLock {
  pthread_spinlock_t spinlock_;

  SpinLock(const SpinLock&);
  SpinLock& operator =(const SpinLock&);
public:
  explicit SpinLock(void)
  {
    if (0 != pthread_spin_init(&spinlock_, 0))
      LOG_FAIL("pthread_spin_init failed\n");
  }

  ~SpinLock(void)
  {
    if (0 != pthread_spin_destroy(&spinlock_))
      LOG_FAIL("pthread_spin_destroy failed\n");
  }

  inline void Lock(void)
  {
    if (0 != pthread_spin_lock(&spinlock_))
      LOG_FAIL("pthread_spin_lock failed\n");
  }

  inline void Unlock(void)
  {
    if (0 != pthread_spin_unlock(&spinlock_))
      LOG_FAIL("pthread_spin_unlock failed\n");
  }
};


#endif  //! __POSIX_LOCKER_HEADER_H__
