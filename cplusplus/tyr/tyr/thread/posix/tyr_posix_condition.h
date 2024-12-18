// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __TYR_POSIX_CONDITION_HEADER_H__
#define __TYR_POSIX_CONDITION_HEADER_H__

namespace tyr {

class Condition : private UnCopyable {
  pthread_cond_t cond_;
public:
  Condition(void) tyr_noexcept {
    TYR_ASSERT(0 == InitCondition());
  }

  ~Condition(void) {
    TYR_ASSERT(0 == pthread_cond_destroy(&cond_));
  }

  void Singal(void) {
    TYR_ASSERT(0 == pthread_cond_signal(&cond_));
  }

  void Broadcast(void) {
    TYR_ASSERT(0 == pthread_cond_broadcast(&cond_));
  }

  void Wait(Mutex& mutex) {
    TYR_ASSERT(0 == pthread_cond_wait(&cond_, mutex.InnerMutex()));
  }

  bool TimedWait(Mutex& mutex, uint64_t timeout) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    timeout += (t.tv_sec * (uint64_t)1e9 + t.tv_nsec);

    t.tv_sec = timeout / NANOSEC;
    t.tv_nsec = timeout % NANOSEC;
    int r = pthread_cond_timedwait(&cond_, mutex.InnerMutex(), &t);
    if (0 == r)
      return true;

    if (ETIMEDOUT == r)
      return false;

    abort();
    return false;
  }
private:
  int InitCondition(void) {
    pthread_condattr_t attr;

    int err = pthread_condattr_init(&attr);
    if (err)
      return -err;

    err = pthread_cond_init(&cond_, &attr);
    if (err)
      goto error2;

    err = pthread_condattr_destroy(&attr);
    if (err)
      goto error;

    return 0;
error:
    pthread_cond_destroy(&cond_);
error2:
    pthread_condattr_destroy(&attr);
    return -err;
  }
};

}

#endif  // __TYR_POSIX_CONDITION_HEADER_H__
