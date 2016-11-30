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
#include <type_traits>
#include <Chaos/OS/OS.h>
#include <Chaos/Concurrent/Unexposed/CurrentThread.h>
#include <Chaos/Concurrent/CurrentThread.h>

namespace Chaos {

namespace CurrentThread {
  static const int kMainTid = Chaos::kern_gettid();
  __declspec(thread) int t_cachaed_tid = 0;
  __declspec(thread) char t_strftid[32];
  __declspec(thread) int t_strftid_length = 12;
  __declspec(thread) const char* t_thread_name = "unknown";
  static_assert(std::is_same<int, pid_t>::value, "pid_t should be `int`");

  namespace Unexposed {
    void set_cached_tid(int cached_tid) {
      CurrentThread::t_cachaed_tid = cached_tid;
    }

    void set_name(const char* name) {
      CurrentThread::t_thread_name = name;
    }
  }

  void cached_tid(void) {
    if (0 == t_cachaed_tid) {
      t_cachaed_tid = Chaos::kern_gettid();
      t_strftid_length = snprintf(t_strftid, sizeof(t_strftid), "%11d ", t_cachaed_tid);
    }
  }

  int get_tid(void) {
    if (__builtin_expect(t_cachaed_tid == 0, 0))
      cached_tid();
    return t_cachaed_tid;
  }

  const char* get_strftid(void) {
    return t_strftid;
  }

  int get_strftid_length(void) {
    return t_strftid_length;
  }

  const char* get_name(void) {
    return t_thread_name;
  }

  bool is_main_thread(void) {
    return get_tid() == kMainTid;
  }

  void sleep_usec(uint64_t usec) {
    // use C++11 chrono on windows
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
  }
}

}
