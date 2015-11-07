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
#ifndef __TYR_POSIX_SEM_HEADER_H__
#define __TYR_POSIX_SEM_HEADER_H__

namespace tyr {

class Semaphore : private UnCopyable {
  sem_t sem_;
public:
  explicit Semaphore(uint32_t value) tyr_noexcept {
    TYR_ASSERT(0 == sem_init(&sem_, 0, value));
  }

  ~Semaphore(void) {
    TYR_ASSERT(0 == sem_destroy(&sem_));
  }

  void Post(void) {
    TYR_ASSERT(0 == sem_post(&sem_));
  }

  void Wait(void) {
    int r;

    do {
      r = sem_wait(&sem_);
    } while (-1 == r && EINTR == errno);

    TYR_ASSERT(0 == r);
  }

  bool TryWait(void) {
    int r;

    do {
      r = sem_trywait(&sem_);
    } while (-1 == r && EINTR == errno);

    if (0 != r) {
      if (EAGAIN == errno)
        return false;
      TYR_ASSERT(0 == r);
    }

    return true;
  }
};

}

#endif  // __TYR_POSIX_SEM_HEADER_H__
