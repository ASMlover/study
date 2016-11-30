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
#ifndef CHAOS_OS_DARWIN_OS_H
#define CHAOS_OS_DARWIN_OS_H

#include <mach/mach_time.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <Chaos/Types.h>

namespace Chaos {

inline int kern_strerror(int errnum, char* buf, size_t buflen) {
  return strerror_r(errnum, buf, buflen);
}

inline int kern_gettimeofday(struct timeval* tv, struct timezone* tz) {
  return gettimeofday(tv, tz);
}

inline pid_t kern_gettid(void) {
  return static_cast<pid_t>(syscall(SYS_thread_selfid));
}

inline int kern_this_thread_setname(const char* name) {
  return pthread_setname_np(name);
}

inline int kern_gettime(struct timespec* timep) {
  mach_timebase_info_data_t info;
  if (KERN_SUCCESS != mach_timebase_info(&info))
    abort();
  uint64_t realtime = mach_absolute_time() * info.numer / info.denom;
  timep->tv_sec = realtime / CHAOS_NANOSEC;
  timep->tv_nsec = realtime % CHAOS_NANOSEC;
  return 0;
}

}

#endif // CHAOS_OS_DARWIN_OS_H
