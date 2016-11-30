// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef CHAOS_CONCURRENT_POSIX_MUTEXBASE_H
#define CHAOS_CONCURRENT_POSIX_MUTEXBASE_H

#include <pthread.h>
#include <Chaos/UnCopyable.h>

namespace Chaos {

class MutexBase : private UnCopyable {
  typedef pthread_mutex_t MutexType;

  mutable MutexType m_;
public:
  MutexBase(void) {
    pthread_mutex_init(&m_, 0);
  }

  ~MutexBase(void) {
    pthread_mutex_destroy(&m_);
  }

  void lock(void) {
    pthread_mutex_lock(&m_);
  }

  bool try_lock(void) {
    return 0 == pthread_mutex_trylock(&m_);
  }

  void unlock(void) {
    pthread_mutex_unlock(&m_);
  }

  MutexType* get_mutex(void) {
    return &m_;
  }
};

}

#endif // CHAOS_CONCURRENT_POSIX_MUTEXBASE_H
