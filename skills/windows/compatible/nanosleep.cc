// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <cstdint>

// nanosleep的精度是100纳秒
void nanosleep(std::uint64_t ns) {
  HANDLE timer;
  if ((timer = CreateWaitableTimer(NULL, TRUE, NULL)) == nullptr)
    return;

  __try {
    LARGE_INTEGER li;
    li.QuadPart = -static_cast<LONGLONG>(ns);
    if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE))
      return;

    WaitForSingleObject(timer, INFINITE);
  }
  __finally {
    CloseHandle(timer);
  }
}

void usleep(std::uint64_t microsec) {
  nanosleep(microsec * 10);
}

static std::int64_t s_freq;
static std::int64_t s_ticker;
void start_ticker(void) {
  LARGE_INTEGER li;
  QueryPerformanceFrequency(&li);

  s_freq = li.QuadPart;
  QueryPerformanceCounter(&li);
  s_ticker = li.QuadPart;
}

std::uint64_t get_interval(void) {
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  return (li.QuadPart - s_ticker) * 1000000000 / s_freq;
}
