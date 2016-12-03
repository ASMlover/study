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
#ifndef CHAOS_OS_WINDOWS_OS_H
#define CHAOS_OS_WINDOWS_OS_H

#include <Windows.h>
#include <string.h>
#include <time.h>
#include <string>

typedef int pid_t;

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

namespace Chaos {

#if !defined(__builtin_expect)
# define __builtin_expect(exp, c) (exp)
#endif

inline errno_t kern_gmtime(const time_t* timep, struct tm* result) {
  return gmtime_s(result, timep);
}

inline errno_t kern_strerror(int errnum, char* buf, size_t buflen) {
  return strerror_s(buf, buflen, errnum);
}

inline time_t kern_timegm(struct tm* timep) {
  return _mkgmtime(timep);
}

inline pid_t kern_getpid(void) {
  return static_cast<pid_t>(GetCurrentProcessId());
}

inline pid_t kern_gettid(void) {
  return static_cast<pid_t>(GetCurrentThreadId());
}

// int kern_getppid(void); // not support on Windows
int kern_gettimeofday(struct timeval* tv, struct timezone* tz);
int kern_this_thread_setname(const char* name);
int kern_backtrace(std::string& bt);

// Windows thread methods wrapper
struct _Thread_t {
  HANDLE notify_start{};
  HANDLE handle{};

  _Thread_t(void) = default;

  _Thread_t(std::nullptr_t)
    : notify_start(nullptr)
    , handle(nullptr) {
  }

  _Thread_t& operator=(std::nullptr_t) {
    notify_start = nullptr;
    handle = nullptr;
    return *this;
  }
};

int kern_thread_create(_Thread_t* thread, void* (*start_routine)(void*), void* arg);

inline int kern_thread_join(_Thread_t thread) {
  if (nullptr != thread.handle) {
    WaitForSingleObject(thread.handle, INFINITE);
    CloseHandle(thread.handle);
  }
  return 0;
}

inline int kern_thread_detach(_Thread_t thread) {
  if (nullptr != thread.handle)
    CloseHandle(thread.handle);
  return 0;
}

inline int kern_thread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
  return 0;
}

// Windows thread local methods wrapper
typedef DWORD _Tls_t;

inline int kern_tls_create(_Tls_t* tls, void (*destructor)(void)) {
  return *tls = FlsAlloc((PFLS_CALLBACK_FUNCTION)destructor), 0;
}

inline int kern_tls_delete(_Tls_t tls) {
  return TRUE == FlsFree(tls) ? 0 : -1;
}

inline int kern_tls_setspecific(_Tls_t tls, const void* value) {
  return TRUE == FlsSetValue(tls, (PVOID)value) ? 0 : -1;
}

inline void* kern_tls_getspecific(_Tls_t tls) {
  return FlsGetValue(tls);
}

}

#endif // CHAOS_OS_WINDOWS_OS_H
