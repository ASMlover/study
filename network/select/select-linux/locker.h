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
#ifndef __LOCKER_HEADER_H__
#define __LOCKER_HEADER_H__

#include <pthread.h>
#include <stdlib.h>

template <typename Locker>
class LockerGuard {
  Locker& locker_;

  LockerGuard(const LockerGuard&);
  LockerGuard& operator =(const LockerGuard&);
public:
  explicit LockerGuard(Locker& locker)
    : locker_(locker)
  {
    locker_.Lock();
  }

  ~LockerGuard(void)
  {
    locker_.Unlock();
  }
};


class Mutex {
  pthread_mutex_t mutex_;

  Mutex(const Mutex&);
  Mutex& operator =(const Mutex&);
public:
  explicit Mutex(void)
  {
    if (0 != pthread_mutex_init(&mutex_, NULL))
      abort();
  }

  ~Mutex(void) 
  {
    if (0 != pthread_mutex_destroy(&mutex_))
      abort();
  }

  inline void Lock(void)
  {
    if (0 != pthread_mutex_lock(&mutex_))
      abort();
  }

  inline void Unlock(void)
  {
    if (0 != pthread_mutex_unlock(&mutex_))
      abort();
  }
};


class SpinLock {
  pthread_spinlock_t spinlock_;

  SpinLock(const SpinLock&);
  SpinLock& operator =(const SpinLock&);
public:
  explicit SpinLock(void)
  {
    if (0 != pthread_spin_init(&spinlock_, NULL))
      abort();
  }

  ~SpinLock(void)
  {
    if (0 != pthread_spin_unlock(&spinlock_))
      abort();
  }

  inline void Lock(void)
  {
    if (0 != pthread_spin_lock(&spinlock_))
      abort();
  }

  inline void Unlock(void)
  {
    if (0 != pthread_spin_unlock(&spinlock_))
      abort();
  }
};

#endif  //! __LOCKER_HEADER_H__
