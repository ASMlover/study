// Copyright (c) 2014 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifndef __MAC_SPINLOCK_HEADER_H__
#define __MAC_SPINLOCK_HEADER_H__

#include <sched.h> // for int sched_yield(void);

typedef int pthread_spinlock_t;
inline int pthread_spin_init(pthread_spinlock_t* lock, int pshared) {
  __asm__ __volatile__ ("" ::: "memory");
  *lock = 0;
  return 0;
}

inline int pthread_spin_destroy(pthread_spinlock_t* lock) {
  return 0;
}

inline int pthread_spin_lock(pthread_spinlock_t* lock) {
  int i;
  while (true) {
    for (i = 0; i < 10000; ++i) {
      if (__sync_bool_compare_and_swap(lock, 0, 1))
        return 0;
    }

    sched_yield();
  }
}

inline int pthread_spin_trylock(pthread_spinlock_t* lock) {
  if (__sync_bool_compare_and_swap(lock, 0, 1))
    return 0;
  return EBUSY;
}

inline int pthread_spin_unlock(pthread_spinlock_t* lock) {
  __asm__ __volatile__ ("" ::: "memory");
  *lock = 0;
  return 0;
}

#include <libkern/OSAtomic.h> // -lc

class Spinlock {
#ifdef __APPLE__
  OSSpinLock spinlock_;
public:
  Spinlock(void)
    : spinlock_(0) {
  }

  inline void Lock(void) {
    OSSpinLockLock(&spinlock_);
  }

  inline bool TryLock(void) {
    return OSSpinLockTry(&spinlock_);
  }

  inline void Unlock(void) {
    OSSpinLockUnlock(&spinlock_);
  }
#else
  pthread_spinlock_t spinlock_;
public:
  Spinlock(void) {
    pthread_spin_init(&spinlock_, 0);
  }

  ~Spinlock(void) {
    pthread_spin_destroy(&spinlock_);
  }

  inline void Lock(void) {
    pthread_spin_lock(&spinlock_);
  }

  inline bool TryLock(void) {
    return EBUSY != pthread_spin_trylock(&spinlock_);
  }

  inline void Unlock(void) {
    pthread_spin_unlock(&spinlock_);
  }
#endif
};

#endif  // __MAC_SPINLOCK_HEADER_H__
