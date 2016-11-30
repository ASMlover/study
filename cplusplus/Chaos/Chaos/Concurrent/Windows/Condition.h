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
#ifndef CHAOS_CONCURRENT_WINDOWS_CONDITION_H
#define CHAOS_CONCURRENT_WINDOWS_CONDITION_H

#include <Windows.h>
#include <Chaos/UnCopyable.h>
#include <Chaos/Concurrent/Mutex.h>

namespace Chaos {

class Condition : private UnCopyable {
  Mutex& mtx_;
  CONDITION_VARIABLE cond_;
public:
  explicit Condition(Mutex& mtx)
    : mtx_(mtx) {
    InitializeConditionVariable(&cond_);
  }

  void wait(void) {
    UnassignScopedMutex guard(mtx_);
    SleepConditionVariableCS(&cond_, mtx_.get_mutex(), INFINITE);
  }

  bool wait_for(int seconds) {
    UnassignScopedMutex guard(mtx_);
    return TRUE == SleepConditionVariableCS(&cond_, mtx_.get_mutex(), static_cast<DWORD>(seconds * 1000));
  }

  void notify_one(void) {
    WakeConditionVariable(&cond_);
  }

  void notify_all(void) {
    WakeAllConditionVariable(&cond_);
  }
};

}

#endif // CHAOS_CONCURRENT_WINDOWS_CONDITION_H
