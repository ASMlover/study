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

static int CondInit(CondVar& cond) {
  cond.waiters_count = 0;
  InitializeCriticalSection(&cond.waiters_count_lock);

  int err;
  do {
    cond.single_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (nullptr == cond.single_event) {
      err = static_cast<int>(GetLastError());
      break;
    }

    cond.broadcast_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (nullptr == cond.broadcast_event) {
      err = static_cast<int>(GetLastError());
      break;
    }

    return 0;
  } while (0);

  if (nullptr != cond.single_event)
    CloseHandle(cond.single_event);
  DeleteCriticalSection(&cond.waiters_count_lock);

  return err;
}

static int CondWaitHelper(
    CondVar& cond, Mutex& mutex, DWORD millitm) {
  HANDLE handles[2] = {cond.single_event, cond.broadcast_event};

  EnterCriticalSection(&cond.waiters_count_lock);
  ++cond.waiters_count;
  LeaveCriticalSection(&cond.waiters_count_lock);

  mutex.Unlock();

  int result = static_cast<int>(
      WaitForMultipleObjects(2, handles, FALSE, millitm));
  bool last_waiter = false;
  EnterCriticalSection(&cond.waiters_count_lock);
  --cond.waiters_count;
  last_waiter = (((WAIT_OBJECT_0 + 1) == result) 
      || (0 == cond.waiters_count));
  LeaveCriticalSection(&cond.waiters_count_lock);

  if (last_waiter)
    ResetEvent(cond.broadcast_event);

  mutex.Lock();

  if ((WAIT_OBJECT_0 == result) || ((WAIT_OBJECT_0 + 1) == result))
    return 0;
  if (WAIT_TIMEOUT == result)
    return result;

  abort();
  return -1;
}


Condition::Condition(Mutex& mutex) 
  : mutex_(mutex) {
  EL_ASSERT(0 == CondInit(cond_));
}

Condition::~Condition(void) {
  EL_ASSERT(CloseHandle(cond_.broadcast_event));
  EL_ASSERT(CloseHandle(cond_.single_event));
  DeleteCriticalSection(&cond_.waiters_count_lock);
}

void Condition::Signal(void) {
  bool have_waiters = false;

  EnterCriticalSection(&cond_.waiters_count_lock);
  have_waiters = cond_.waiters_count > 0;
  LeaveCriticalSection(&cond_.waiters_count_lock);

  if (have_waiters)
    SetEvent(cond_.single_event);
}

void Condition::SignalAll(void) {
  bool have_waiters = false;

  EnterCriticalSection(&cond_.waiters_count_lock);
  have_waiters = cond_.waiters_count > 0;
  LeaveCriticalSection(&cond_.waiters_count_lock);

  if (have_waiters)
    SetEvent(cond_.broadcast_event);
}

void Condition::Wait(void) {
  EL_ASSERT(0 == CondWaitHelper(cond_, mutex_, INFINITE));
}

void Condition::TimedWait(uint64_t timeout) {
  EL_ASSERT(0 == CondWaitHelper(cond_, mutex_, timeout));
}

}
