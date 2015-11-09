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
#ifndef __TYR_WIN_SEM_HEADER_H__
#define __TYR_WIN_SEM_HEADER_H__

namespace tyr {

class Semaphore : private UnCopyable {
  HANDLE sem_ = nullptr;
public:
  explicit Semaphore(uint32_t value) tyr_noexcept
    : sem_(CreateSemaphore(nullptr, value, INT_MAX, nullptr)) {
    TYR_ASSERT(nullptr != sem_);
  }

  ~Semaphore(void) {
    TYR_ASSERT(CloseHandle(sem_));
  }

  void Post(void) {
    TYR_ASSERT(ReleaseSemaphore(sem_, 1, nullptr));
  }

  void Wait(void) {
    TYR_ASSERT(WAIT_OBJECT_0 == WaitForSingleObject(sem_, INFINITE));
  }

  bool TryWait(void) {
    return WAIT_OBJECT_0 == WaitForSingleObject(sem_, 0);
  }
};

}

#endif  // __TYR_WIN_SEM_HEADER_H__
