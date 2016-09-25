/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <assert.h>
#include "../sl_cond.h"

#if defined(_WINDOWS_) || defined(_MSC_VER)
  #define inline __inline
#endif



static inline int 
sl_cond_wait_helper(sl_cond_t* cond, DWORD millitm)
{
  int ret;
  int last_waiter = 0;
  HANDLE handles[2] = {cond->cond.signal_event, cond->cond.broadcast_event};

  EnterCriticalSection(&cond->cond.waiters_count_lock);
  ++cond->cond.waiters_count;
  LeaveCriticalSection(&cond->cond.waiters_count_lock);

  sl_mutex_unlock(cond->mutex);

  ret = WaitForMultipleObjects(2, handles, FALSE, millitm);

  EnterCriticalSection(&cond->cond.waiters_count_lock);
  --cond->cond.waiters_count;
  last_waiter = (((WAIT_OBJECT_0 + 1) == ret) 
      && (0 == cond->cond.waiters_count));
  LeaveCriticalSection(&cond->cond.waiters_count_lock);

  if (last_waiter)
    ResetEvent(cond->cond.broadcast_event);

  sl_mutex_lock(cond->mutex);

  if ((WAIT_OBJECT_0 == ret) || ((WAIT_OBJECT_0 + 1) == ret))
    return 0;
  if (WAIT_TIMEOUT == ret)
    return ret;

  abort();
  return -1;
}




int 
sl_cond_init(sl_cond_t* cond, sl_mutex_t* mutex)
{
  int err;

  cond->mutex = mutex;
  cond->cond.waiters_count = 0;

  InitializeCriticalSection(&cond->cond.waiters_count_lock);
  cond->cond.signal_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (NULL == cond->cond.signal_event) {
    err = (int)GetLastError();
    goto Exit2;
  }

  cond->cond.broadcast_event = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (NULL == cond->cond.broadcast_event) {
    err = (int)GetLastError();
    goto Exit;
  }

  return 0;

Exit:
  CloseHandle(cond->cond.signal_event);
Exit2:
  DeleteCriticalSection(&cond->cond.waiters_count_lock);
  return err;
}

void 
sl_cond_destroy(sl_cond_t* cond)
{
  if (!CloseHandle(cond->cond.broadcast_event))
    abort();
  if (!CloseHandle(cond->cond.signal_event))
    abort();
  DeleteCriticalSection(&cond->cond.waiters_count_lock);
}

void 
sl_cond_signal(sl_cond_t* cond)
{
  int have_waiters = 0;

  EnterCriticalSection(&cond->cond.waiters_count_lock);
  have_waiters = cond->cond.waiters_count > 0;
  LeaveCriticalSection(&cond->cond.waiters_count_lock);

  if (have_waiters)
    SetEvent(cond->cond.signal_event);
}

void 
sl_cond_broadcast(sl_cond_t* cond)
{
  int have_waiters = 0;

  EnterCriticalSection(&cond->cond.waiters_count_lock);
  have_waiters = cond->cond.waiters_count > 0;
  LeaveCriticalSection(&cond->cond.waiters_count_lock);

  if (have_waiters)
    SetEvent(cond->cond.broadcast_event);
}

void 
sl_cond_wait(sl_cond_t* cond)
{
  if (0 != sl_cond_wait_helper(cond, INFINITE))
    abort();
}

int 
sl_cond_timedwait(sl_cond_t* cond, unsigned int millitm)
{
  return sl_cond_wait_helper(cond, (DWORD)millitm);
}
