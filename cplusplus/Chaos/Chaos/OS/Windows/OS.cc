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
#include <stdint.h>
#include <stdio.h>
#include <Chaos/OS/Windows/OS.h>

namespace Chaos {

static const uint64_t kEpoch = 116444736000000000ULL;
static const DWORD kMSVCException = 0x406D1388;
static const int kMaxBacktrace = 256;
static const HANDLE kMainProc = GetCurrentProcess();
static const BOOL kBTIgnoredInit = SymInitialize(kMainProc, NULL, TRUE);

#pragma pack(push, 8)
typedef struct KernThreadName {
  DWORD dwType; // must be 0x1000
  LPCSTR szName;
  DWORD dwThreadID; // thread id
  DWORD dwFlags;
} KernThreadName;
#pragma pack(pop)

int kern_gettimeofday(struct timeval* tv, struct timezone* /*tz*/) {
  if (nullptr != tv) {
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

int kern_this_thread_setname(const char* name) {
  KernThreadName tn;
  tn.dwType = 0x1000;
  tn.szName = name;
  tn.dwThreadID = GetCurrentThreadId();
  tn.dwFlags = 0;
  __try {
    RaiseException(kMSVCException, 0, sizeof(tn) / sizeof(ULONG_PTR), (ULONG_PTR*)&tn);
  }
  __except (EXCEPTION_EXECUTE_HANDLER) {
  }
  return 0;
}

int kern_backtrace(std::string& bt) {
  void* stack[kMaxBacktrace];
  int frames = CaptureStackBackTrace(0, kMaxBacktrace, stack, NULL);

  char symbol_buff[sizeof(SYMBOL_INFO) + kMaxBacktrace * sizeof(char)];
  PSYMBOL_INFO symbol = (PSYMBOL_INFO)symbol_buff;
  symbol->MaxNameLen = kMaxBacktrace;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  char message[1024];
  for (int i = 0; i < frames; ++i) {
    SymFromAddr(kMainProc, (DWORD)stack[i], 0, symbol);
    snprintf(message, sizeof(message), "%i: %s - 0x%p\n", frames - i - 1, symbol->Name, (void*)symbol->Address);
    bt.append(message);
  }

  return 0;
}

}
