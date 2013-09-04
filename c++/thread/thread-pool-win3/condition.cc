//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include "noncopyable.h"
#include "mutex.h"
#include "thread_guard.h"
#include "condition.h"





condition_t::condition_t(mutex_t& mutex) 
  : mutex_(mutex)
{
  if (0 != cond_init(&cond_))
    abort();
}

condition_t::~condition_t(void)
{
  cond_destroy(&cond_);
}

void 
condition_t::wait(void)
{
  thread_guard_t lock(mutex_);
  cond_wait(&cond_, &mutex_);
}

bool 
condition_t::timedwait(unsigned int millitm)
{
  thread_guard_t lock(mutex_);
  return (WAIT_TIMEOUT == cond_timedwait(&cond_, &mutex_, millitm));
}

void 
condition_t::notify(void)
{
  cond_signal(&cond_);
}

void 
condition_t::notify_all(void)
{
  cond_boardcast(&cond_);
}






int 
condition_t::cond_init(cond_t* cond)
{
  cond->waiters_count = 0;
  InitializeCriticalSection(&cond->waiters_count_lock);

  int err;
  cond->signal_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (NULL == cond->signal_event) {
    err = GetLastError();
    goto Exit2;
  }

  cond->boardcast_event = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (NULL == cond->boardcast_event) {
    err = GetLastError();
    goto Exit;
  }

  return 0;

Exit:
  CloseHandle(cond->signal_event);
Exit2:
  DeleteCriticalSection(&cond->waiters_count_lock);
  return err;
}

void 
condition_t::cond_destroy(cond_t* cond)
{
  if (!CloseHandle(cond->boardcast_event))
    abort();
  if (!CloseHandle(cond->signal_event))
    abort();
  DeleteCriticalSection(&cond->waiters_count_lock);
}

void 
condition_t::cond_signal(cond_t* cond)
{
  bool have_waiters = false;

  EnterCriticalSection(&cond->waiters_count_lock);
  have_waiters = cond->waiters_count > 0;
  LeaveCriticalSection(&cond->waiters_count_lock);

  if (have_waiters)
    SetEvent(cond->signal_event);
}

void 
condition_t::cond_boardcast(cond_t* cond)
{
  bool have_waiters = 0;

  EnterCriticalSection(&cond->waiters_count_lock);
  have_waiters = cond->waiters_count > 0;
  LeaveCriticalSection(&cond->waiters_count_lock);

  if (have_waiters)
    SetEvent(cond->boardcast_event);
}

void 
condition_t::cond_wait(cond_t* cond, mutex_t* mutex)
{
  if (0 != wait_helper(cond, mutex, INFINITE))
    abort();
}

int 
condition_t::cond_timedwait(cond_t* cond, 
    mutex_t* mutex, unsigned int timeout)
{
  return wait_helper(cond, mutex, timeout);
}


int 
condition_t::wait_helper(cond_t* cond, mutex_t* mutex, unsigned int timeout)
{
  HANDLE handles[2] = {cond->signal_event, cond->boardcast_event};

  EnterCriticalSection(&cond->waiters_count_lock);
  ++cond->waiters_count;
  LeaveCriticalSection(&cond->waiters_count_lock);

  mutex->unlock();

  DWORD ret = WaitForMultipleObjects(2, handles, FALSE, (DWORD)timeout);

  bool last_waiter = false;
  EnterCriticalSection(&cond->waiters_count_lock);
  --cond->waiters_count;
  last_waiter = ((WAIT_OBJECT_0 + 1 == ret) && (0 == cond->waiters_count));
  LeaveCriticalSection(&cond->waiters_count_lock);

  if (last_waiter)
    ResetEvent(cond->boardcast_event);

  mutex->lock();
  if (WAIT_OBJECT_0 == ret || WAIT_OBJECT_0 + 1 == ret)
    return 0;

  if (WAIT_TIMEOUT == ret)
    return ret;

  abort();
  return -1;
}
