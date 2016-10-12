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
#include <process.h>
#include <assert.h>
#include <stdlib.h>
#include "../TPlatform.h"

namespace tyr { namespace basic {

static const uint64_t kEpoch = 116444736000000000ULL;
static const DWORD kMSVCException = 0x406D1388;

#pragma pack(push, 8)
typedef struct THREADNAME_INFO {
  DWORD dwType; // must be 0x1000
  LPCSTR szName;
  DWORD dwThreadID; // thread id
  DWORD dwFlags;
} THREADNAME_INFO;
#pragma pack(pop)

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

int kern_mutex_init(kern_mutex_t* mtx) {
  return InitializeCriticalSection(mtx), 0;
}

int kern_mutex_destroy(kern_mutex_t* mtx) {
  return DeleteCriticalSection(mtx), 0;
}

int kern_mutex_lock(kern_mutex_t* mtx) {
  if ((DWORD)mtx->OwningThread != GetCurrentThreadId())
    EnterCriticalSection(mtx);
  return 0;
}

int kern_mutex_unlock(kern_mutex_t* mtx) {
  return LeaveCriticalSection(mtx), 0;
}

int kern_this_thread_setname(const char* name) {
  THREADNAME_INFO ti;
  ti.dwType = 0x1000;
  ti.szName = name;
  ti.dwThreadID = GetCurrentThreadId();
  ti.dwFlags = 0;
  __try {
    RaiseException(kMSVCException, 0, sizeof(ti) / sizeof(ULONG_PTR), (ULONG_PTR*)&ti);
  }
  __except (EXCEPTION_EXECUTE_HANDLER) {
  }
  return 0;
}

int kern_cond_init(kern_cond_t* cond) {
  return InitializeConditionVariable(cond), 0;
}

int kern_cond_destroy(kern_cond_t* /*cond*/) {
  return 0;
}

int kern_cond_signal(kern_cond_t* cond) {
  return WakeConditionVariable(cond), 0;
}

int kern_cond_broadcast(kern_cond_t* cond) {
  return WakeAllConditionVariable(cond), 0;
}

int kern_cond_wait(kern_cond_t* cond, kern_mutex_t* mtx) {
  return SleepConditionVariableCS(cond, mtx, INFINITE) ? 0 : -1;
}

int kern_cond_timedwait(kern_cond_t* cond, kern_mutex_t* mtx, uint64_t nanosec) {
  return SleepConditionVariableCS(cond, mtx, static_cast<DWORD>(nanosec / 1e6)) ? 0 : -1;
}

int kern_thread_create(kern_thread_t* thread, kern_start_routine_t start_routine, void* arg) {
  thread->start_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  assert(nullptr != thread->start_event);

  thread->thrd_handle = (HANDLE)_beginthreadex(nullptr, 0, start_routine, arg, 0, nullptr);
  assert(nullptr != thread->thrd_handle);

  WaitForSingleObject(thread->start_event, INFINITE);
  CloseHandle(thread->start_event);

  return 0;
}

int kern_thread_created_signal(kern_thread_t* thread) {
  return SetEvent(thread->start_event), 0;
}

int kern_thread_join(kern_thread_t* thread) {
  if (nullptr != thread->thrd_handle) {
    WaitForSingleObject(thread->thrd_handle, INFINITE);
    CloseHandle(thread->thrd_handle);

    thread->reset();
  }

  return 0;
}

}}
