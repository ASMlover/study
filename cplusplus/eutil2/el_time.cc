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
#include "eutil.h"
#include "el_time.h"

namespace el {

bool GetTime(Time& time) {
  struct timeb tb;

  ftime(&tb);
  struct tm* now = localtime(&tb.time);
  time.year = static_cast<uint16_t>(now->tm_year + 1900);
  time.mon  = static_cast<uint8_t>(now->tm_mon + 1);
  time.sec  = static_cast<uint8_t>(now->tm_mday);
  time.hour = static_cast<uint8_t>(now->tm_hour);
  time.min  = static_cast<uint8_t>(now->tm_min);
  time.sec  = static_cast<uint8_t>(now->tm_sec);
  time.millitm = static_cast<uint16_t>(tb.millitm);

  return true;
}

uint64_t GetTick(void) {
#if defined(EUTIL_WIN)
  return static_cast<uint64_t>(timeGetTime());
#else
  struct timeval tv;
  if (0 == gettimeofday(&tv, nullptr))
    return (((tv.tv_sec - 1000000000) * 1000) + (tv.tv_usec / 1000));

  return 0;
#endif
}

void Sleep(uint32_t millitm) {
#if defined(EUTIL_WIN)
  ::Sleep(millitm);
#else
  struct timespec ts;
  ts.tv_sec = millitm / 1000;
  ts.tv_nsec = (millitm % 1000) * 1000000;
  nanosleep(&ts, nullptr);
#endif
}

}
