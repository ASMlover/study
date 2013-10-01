#include <stdlib.h>
#include "mutex.h"
#include "condition.h"


static inline int 
CondVarInit(CondVar* cond)
{
  cond->waiters_count = 0;
  InitializeCriticalSection(&cond->waiters_count_lock);

  int err;
  cond->signal_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (NULL == cond->signal_event) {
    err = (int)GetLastError();
    goto Exit2;
  }

  cond->broadcast_event = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (NULL == cond->broadcast_event) {
    err = (int)GetLastError();
    goto Exit;
  }

  return 0;

Exit:
  CloseHandle(cond->signal_event);
Exit2:
  DeleteCriticalSection(&cond->waiters_count_lock);
  return err;
}

static inline int 
CondVarWaitHelper(CondVar* cond, Mutex* mutex, DWORD millitm)
{
  HANDLE handles[2] = {cond->signal_event, cond->broadcast_event};

  EnterCriticalSection(&cond->waiters_count_lock);
  ++cond->waiters_count;
  LeaveCriticalSection(&cond->waiters_count_lock);

  mutex->Unlock();

  int ret = (int)WaitForMultipleObjects(2, handles, FALSE, millitm);

  bool last_waiter = false;
  EnterCriticalSection(&cond->waiters_count_lock);
  --cond->waiters_count;
  last_waiter = ((WAIT_OBJECT_0 + 1) == ret && 0 == cond->waiters_count);
  LeaveCriticalSection(&cond->waiters_count_lock);

  if (last_waiter)
    ResetEvent(cond->broadcast_event);

  mutex->Lock();

  if (WAIT_OBJECT_0 == ret || WAIT_OBJECT_0 + 1 == ret)
    return 0;
  if (WAIT_TIMEOUT == ret)
    return ret;

  abort();
  return -1;
}





Condition::Condition(Mutex& mutex)
  : mutex_(mutex_)
{
  if (0 != CondVarInit(&cond_))
    abort();
}

Condition::~Condition(void)
{
  if (!CloseHandle(cond_.broadcast_event))
    abort();
  if (!CloseHandle(cond_.signal_event))
    abort();
  DeleteCriticalSection(&cond_.waiters_count_lock);
}

void 
Condition::Signal(void)
{
  bool have_waiters = false;

  EnterCriticalSection(&cond_.waiters_count_lock);
  have_waiters = cond_.waiters_count > 0;
  LeaveCriticalSection(&cond_.waiters_count_lock);

  if (have_waiters)
    SetEvent(cond_.signal_event);
}

void 
Condition::SignalAll(void)
{
  bool have_waiters = false;

  EnterCriticalSection(&cond_.waiters_count_lock);
  have_waiters = cond_.waiters_count > 0;
  LeaveCriticalSection(&cond_.waiters_count_lock);

  if (have_waiters)
    SetEvent(cond_.broadcast_event);
}

void 
Condition::Wait(void)
{
  if (0 != CondVarWaitHelper(&cond_, &mutex_, INFINITE))
    abort();
}

int 
Condition::TimedWait(DWORD millitm)
{
  return CondVarWaitHelper(&cond_, &mutex_, INFINITE);
}
