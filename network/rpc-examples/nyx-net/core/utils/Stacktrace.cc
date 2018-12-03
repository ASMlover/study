// Copyright (c) 2018 ASMlover. All rights reserved.
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
#if defined(_WINDOWS_) || defined(_MSC_VER)
# include <Windows.h>
# include <DbgHelp.h>
#else
# include <execinfo.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <core/utils/Stacktrace.h>

namespace nyx::utils {

static constexpr int kMaxStacktrace = 256;

#if defined(_WINDOWS_) || defined(_MSC_VER)

void print_stacktrace(void) {
  HANDLE hProc = GetCurrentProcess();
  SymInitialize(hProc, NULL, TRUE);

  void* stack[kMaxStacktrace];
  int frames = CaptureStackBackTrace(0, kMaxStacktrace, stack, NULL);

  char symbol_buff[sizeof(SYMBOL_INFO) * kMaxStacktrace * sizeof(char)];
  PSYMBOL_INFO symbol = (PSYMBOL_INFO)symbol_buff;
  symbol->MaxNameLen = kMaxStacktrace;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  for (int i = 0; i < frames; ++i) {
    SymFromAddr(hProc, *reinterpret_cast<DWORD*>(stack[i]), 0, symbol);
    std::cout
      << frames - i - 1 << ": " << symbol->Name << " - "
      << (void*)symbol->Address << std::endl;
  }
}
#else
void print_stacktrace(void) {
  void* buff[kMaxStacktrace];
  int nptrs = backtrace(buff, kMaxStacktrace);
  char** messages = backtrace_symbols(buff, nptrs);
  if (messages) {
    for (int i = 0; i < nptrs; ++i)
        std::cout << nptrs - i - 1 << ": " << messages[i] << std::endl;
    std::free(messages);
  }
}
#endif

}
