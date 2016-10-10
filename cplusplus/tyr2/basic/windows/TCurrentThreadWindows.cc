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
#include <stdio.h>
#include <chrono>
#include <thread>
#include "../TPlatform.h"
#include "../TTimestamp.h"
#include "../unexposed/TCurrentThreadUnexposed.h"
#include "../TCurrentThread.h"

namespace tyr { namespace basic {

namespace CurrentThread {
  static const int kMainTid = kern_gettid();
  __declspec(thread) int tCachedTid = 0;
  __declspec(thread) char tTidString[32];
  __declspec(thread) int tTidStringLength = 12;
  __declspec(thread) const char* tThreadName = "unknown";
  static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

  namespace unexposed {
    void set_cached_tid(int cached_tid) {
      CurrentThread::tCachedTid = cached_tid;
    }

    void set_thread_name(const char* name) {
      CurrentThread::tThreadName = name;
    }
  }

  void cached_tid(void) {
    if (0 == tCachedTid) {
      tCachedTid = kern_gettid();
      tTidStringLength = snprintf(tTidString, sizeof(tTidString), "%11d ", tCachedTid);
    }
  }

  int tid(void) {
    if (__builtin_expect(tCachedTid == 0, 0))
      cached_tid();
    return tCachedTid;
  }

  const char* tid_string(void) {
    return tTidString;
  }

  int tid_string_length(void) {
    return tTidStringLength;
  }

  const char* name(void) {
    return tThreadName;
  }

  bool is_main_thread(void) {
    return tid() == kMainTid;
  }

  void sleep_usec(int64_t usec) {
    // use C++11 on Windows
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
  }

}}}
