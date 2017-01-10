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
#include <Windows.h>
#include <DbgHelp.h>
#include <process.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "../TPlatform.h"

namespace tyr { namespace basic {

static const uint64_t kEpoch = 116444736000000000ULL;
static const DWORD kMSVCException = 0x406D1388;
static const int kMaxBackTrace = 256;
static const HANDLE kMainProc = GetCurrentProcess();
static const BOOL kBTTempInit = SymInitialize(kMainProc, NULL, TRUE);

#pragma pack(push, 8)
typedef struct KernThreadName {
  DWORD dwType; // must be 0x1000
  LPCSTR szName;
  DWORD dwThreadID; // thread id
  DWORD dwFlags;
} KernThreadName;
#pragma pack(pop)

typedef struct KernThreadParams {
  KernThread* thread;
  void* (*start)(void*);
  void* arg;
} KernThreadParams;

int gettimeofday(struct timeval* tv, struct timezone* /*tz*/) {
  if (tv) {
    FILETIME ft;
    SYSTEMTIME st;
    ULARGE_INTEGER uli;

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    tv->tv_sec = static_cast<long>((uli.QuadPart - kEpoch) / 10000000L);
    tv->tv_usec = static_cast<long>(st.wMilliseconds * 1000);
  }
  return 0;
}

pid_t kern_getpid(void) {
  return static_cast<pid_t>(GetCurrentProcessId());
}

pid_t kern_gettid(void) {
  return static_cast<pid_t>(GetCurrentThreadId());
}

int kern_backtrace(std::string& bt) {
  void* stack[kMaxBackTrace];
  int frames = CaptureStackBackTrace(0, kMaxBackTrace, stack, nullptr);

  char symbol_buff[sizeof(SYMBOL_INFO) + kMaxBackTrace * sizeof(char)];
  PSYMBOL_INFO symbol = (PSYMBOL_INFO)symbol_buff;
  symbol->MaxNameLen = kMaxBackTrace;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  char message[1024];
  for (int i = 0; i < frames; ++i) {
    SymFromAddr(kMainProc, (DWORD)stack[i], 0, symbol);
    snprintf(message, sizeof(message), "%i: %s - 0x%p\n", frames - i - 1, symbol->Name, (void*)symbol->Address);
    bt.append(message);
  }

  return 0;
}

enum {
  KMUTEX_SUCCESS,
  KMUTEX_TIMEDOUT,
  KMUTEX_BUSY,
  KMUTEX_ERROR,
};

int kern_mutex_do_lock(KernMutex* mtx, bool try_lock = false) {
  int r = WAIT_TIMEOUT;
  pid_t tid = kern_gettid();

  if (!try_lock) {
    if (mtx->tid != tid) {
      mtx->lock();
      mtx->tid = tid;
    }
    ++mtx->count;
    return KMUTEX_SUCCESS;
  }
  else {
    if (!try_lock) {
      if (mtx->tid != tid)
        mtx->lock();
      r = WAIT_OBJECT_0;
    }
    else {
      if (mtx->tid != tid)
        r = mtx->try_lock() ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
      else
        r = WAIT_OBJECT_0;
    }

    if (r != WAIT_OBJECT_0 && r != WAIT_ABANDONED)
      ;
    else if (1 < ++mtx->count)
      r = (--mtx->count, WAIT_TIMEOUT);
    else
      mtx->tid = tid;

    switch (r) {
      case WAIT_OBJECT_0:
      case WAIT_ABANDONED:
        return KMUTEX_SUCCESS;
      case WAIT_TIMEOUT:
        return try_lock ? KMUTEX_BUSY : KMUTEX_TIMEDOUT;
      default:
        return KMUTEX_ERROR;
    }
  }

  return KMUTEX_ERROR;
}

inline int kern_mutex_check_return(int r) {
  if (r != KMUTEX_SUCCESS)
    abort();
  return r;
}

inline int kern_mutex_check_return(int r, int other) {
  if (r != KMUTEX_SUCCESS && r != other)
    abort();
  return r;
}

int kern_mutex_init(KernMutex* /*mtx*/) {
  return KMUTEX_SUCCESS;
}

int kern_mutex_destroy(KernMutex* /*mtx*/) {
  return KMUTEX_SUCCESS;
}

int kern_mutex_lock(KernMutex* mtx) {
  return kern_mutex_check_return(kern_mutex_do_lock(mtx));
}

int kern_mutex_unlock(KernMutex* mtx) {
  if (0 == --mtx->count) {
    mtx->tid = -1;
    mtx->unlock();
  }
  return KMUTEX_SUCCESS;
}

int kern_this_thread_setname(const char* name) {
  KernThreadName ni;
  ni.dwType = 0x1000;
  ni.szName = name;
  ni.dwThreadID = GetCurrentThreadId();
  ni.dwFlags = 0;
  __try {
    RaiseException(kMSVCException, 0, sizeof(ni) / sizeof(ULONG_PTR), (ULONG_PTR*)&ni);
  }
  __except (EXCEPTION_EXECUTE_HANDLER) {
  }
  return 0;
}

int kern_cond_init(KernCond* cond) {
  return InitializeConditionVariable(cond), 0;
}

int kern_cond_destroy(KernCond* /*cond*/) {
  return 0;
}

int kern_cond_signal(KernCond* cond) {
  return WakeConditionVariable(cond), 0;
}

int kern_cond_broadcast(KernCond* cond) {
  return WakeAllConditionVariable(cond), 0;
}

int kern_cond_wait(KernCond* cond, KernMutex* mtx) {
  return SleepConditionVariableCS(cond, (CRITICAL_SECTION*)mtx, INFINITE) ? 0 : -1;
}

int kern_cond_timedwait(KernCond* cond, KernMutex* mtx, uint64_t nanosec) {
  return SleepConditionVariableCS(cond, (CRITICAL_SECTION*)mtx, static_cast<DWORD>(nanosec / 1e6)) ? 0 : -1;
}

UINT WINAPI kern_thread_start_routine(void* arg) {
  std::unique_ptr<KernThreadParams> params(static_cast<KernThreadParams*>(arg));
  if (!params)
    return 0;

  SetEvent(params->thread->start_event);
  if (nullptr != params->start)
    params->start(params->arg);

  return 0;
}

int kern_thread_create(KernThread* thread, void* (*start_routine)(void*), void* arg) {
  thread->start_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (nullptr == thread->start_event)
    return -1;

  int result = -1;
  std::unique_ptr<KernThreadParams> params(new KernThreadParams);
  if (!params)
    goto _Exit;
  params->thread = thread;
  params->start = start_routine;
  params->arg = arg;

  thread->thrd_handle = reinterpret_cast<HANDLE>(_beginthreadex(
        nullptr, 0, kern_thread_start_routine, params.get(), 0, nullptr));
  if (nullptr == thread->thrd_handle)
    goto _Exit;

  WaitForSingleObject(thread->start_event, INFINITE);
  params.release();
  result = 0;

_Exit:
  CloseHandle(thread->start_event);
  thread->start_event = nullptr;
  return result;
}

int kern_thread_join(KernThread thread) {
  if (nullptr != thread.thrd_handle) {
    WaitForSingleObject(thread.thrd_handle, INFINITE);
    CloseHandle(thread.thrd_handle);
  }
  return 0;
}

int kern_thread_detach(KernThread thread) {
  if (nullptr != thread.thrd_handle)
    CloseHandle(thread.thrd_handle);
  return 0;
}

int kern_thread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
  return 0;
}

int kern_threadkey_create(KernThreadKey* key, void (*destructor)(void*)) {
  return *key = FlsAlloc((PFLS_CALLBACK_FUNCTION)destructor), 0;
}

int kern_threadkey_delete(KernThreadKey key) {
  return TRUE == FlsFree(key) ? 0 : -1;
}

int kern_setspecific(KernThreadKey key, const void* value) {
  return TRUE == FlsSetValue(key, (PVOID)value) ? 0 : -1;
}

void* kern_getspecific(KernThreadKey key) {
  return FlsGetValue(key);
}

}}
