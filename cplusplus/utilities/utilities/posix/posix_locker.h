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
#ifndef __UTIL_POSIX_LOCKER_HEADER_H__
#define __UTIL_POSIX_LOCKER_HEADER_H__

namespace util {

class Mutex : private UnCopyable {
  pthread_mutex_t mutex_;
public:
  Mutex(void) {
    UTIL_ASSERT(0 == pthread_mutex_init(&mutex_, NULL));
  }

  ~Mutex(void) {
    UTIL_ASSERT(0 == pthread_mutex_destroy(&mutex_));
  }

  void Lock(void) {
    UTIL_ASSERT(0 == pthread_mutex_lock(&mutex_));
  }

  void Unlock(void) {
    UTIL_ASSERT(0 == pthread_mutex_unlock(&mutex_));
  }
};

class SpinLock : private UnCopyable {
  pthread_spinlock_t spinlock_;
public:
  SpinLock(void) {
    UTIL_ASSERT(0 == pthread_spin_init(&spinlock_, 0));
  }

  ~SpinLock(void) {
    UTIL_ASSERT(0 == pthread_spin_destroy(&spinlock_));
  }

  void Lock(void) {
    UTIL_ASSERT(0 == pthread_spin_lock(&spinlock_));
  }

  void Unlock(void) {
    UTIL_ASSERT(0 == pthread_spin_unlock(&spinlock_));
  }
};

}

#endif  // __UTIL_POSIX_LOCKER_HEADER_H__
