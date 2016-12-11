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
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include "../../basic/TLogging.h"
#include "../TKernWrapper.h"

namespace tyr { namespace net {

namespace Kern {
  int create_timer(void) {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
      TYRLOG_SYSFATAL << "Failed in timerfd_create";

    return timerfd;
  }

  void close_timer(int timerfd) {
    close(timerfd);
  }

  int read_timer(int timerfd, void* buf, size_t len) {
    ssize_t n = read(timerfd, buf, len);
    return static_cast<int>(n);
  }

  int set_timer(int timerfd, int64_t msec) {
    struct itimerspec oldt{};
    struct itimerspec newt{};

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(msec / 1000000);
    ts.tv_nsec = static_cast<long>((msec % 1000000) * 1000);
    newt.it_value = ts;
    int r = timerfd_settime(timerfd, 0, &newt, &oldt);
    if (r)
      TYRLOG_SYSERR << "timerfd_settime";
    return 0;
  }
}

}}
