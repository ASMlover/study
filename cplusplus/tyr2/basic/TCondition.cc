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
#include <errno.h>
#include <time.h>
#include "TConfig.h"
#if defined(TYR_DARWIN)
# include <mach/mach_time.h>
#endif
#include "TTypes.h"
#include "TCondition.h"

namespace tyr {

#undef NANOSEC
#define NANOSEC ((uint64_t)1e9)

bool Condition::timed_wait(int seconds) {
  struct timespec ts;
#if defined(TYR_LINUX)
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += seconds;
#elif defined(TYR_DARWIN)
  mach_timebase_info_data_t info;
  if (KERN_SUCCESS != mach_timebase_info(&info))
    abort();
  uint64_t hrtime = mach_absolute_time() * info.numer / info.denom;
  ts.tv_sec = hrtime / NANOSEC + seconds;
  ts.tv_nsec = hrtime % NANOSEC;
#else
# error ">>>>>>>>>> Unknown platform >>>>>>>>>>"
#endif

  Mutex::UnassignedGuard guard(mtx_);
  return ETIMEDOUT == pthread_cond_timedwait(&cond_, mtx_.get_mutex(), &ts);
}

}
