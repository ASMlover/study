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
#include "../eutil.h"
#include "../el_condition.h"

namespace el {

#undef NANOSEC
#define NANOSEC ((uint64_t)1e9)

static inline uint64_t hrtime(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (((uint64_t)ts.tv_sec) * NANOSEC + ts.tv_nsec);
}

Condition::Condition(Mutex& mutex) 
  : mutex_(mutex) {
  EL_ASSERT(0 == pthread_cond_init(&cond_, 0));
}

Condition::~Condition(void) {
  EL_ASSERT(0 == pthread_cond_destroy(&cond_));
}

void Condition::Signal(void) {
  EL_ASSERT(0 == pthread_cond_signal(&cond_));
}

void Condition::SignalAll(void) {
  EL_ASSERT(0 == pthread_cond_broadcast(&cond_));
}

void Condition::Wait(void) {
  EL_ASSERT(0 == pthread_cond_wait(&cond_, mutex_.mutex()));
}

void Condition::TimedWait(uint64_t timeout) {
  struct timespec ts;

  timeout += hrtime();
  ts.tv_sec = timeout / NANOSEC;
  ts.tv_nsec = timeout % NANOSEC;

  EL_ASSERT(0 == pthread_cond_timedwait(&cond_, mutex_.mutex(), &ts));
}

}
