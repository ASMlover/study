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
#ifndef __EL_WIN_LOCKER_HEADER_H__
#define __EL_WIN_LOCKER_HEADER_H__

namespace el {

class Mutex : private UnCopyable {
  CRITICAL_SECTION mutex_;
public:
  Mutex(void) {
    InitializeCriticalSection(&mutex_);
  }

  ~Mutex(void) {
    DeleteCriticalSection(&mutex_);
  }

  inline void Lock(void) {
    EnterCriticalSection(&mutex_);
  }

  inline void Unlock(void) {
    LeaveCriticalSection(&mutex_);
  }

  inline CRITICAL_SECTION* mutex(void) const {
    return const_cast<CRITICAL_SECTION*>(&mutex_);
  }
};

class SpinLock : private UnCopyable {
  CRITICAL_SECTION spinlock_;
public:
  SpinLock(void) {
    EL_ASSERT(InitializeCriticalSectionAndSpinCount(&spinlock_, 4000));
  }

  ~SpinLock(void) {
    DeleteCriticalSection(&spinlock_);
  }

  inline void Lock(void) {
    EnterCriticalSection(&spinlock_);
  }

  inline void Unlock(void) {
    LeaveCriticalSection(&spinlock_);
  }
};

}

#endif  // __EL_WIN_LOCKER_HEADER_H__
