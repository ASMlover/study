// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __TYR_CONDITION_HEADER_H__
#define __TYR_CONDITION_HEADER_H__

#if defined(TYR_CPP0X)

namespace tyr {

class Condition : private UnCopyable {
  std::condition_variable cond_;
public:
  void Singal(void) {
    cond_.notify_one();
  }

  void Broadcast(void) {
    cond_.notify_all();
  }

  void Wait(Mutex& mutex) {
    std::unique_lock<std::mutex> lock(*mutex.InnerMutex());
    cond_.wait(lock);
  }

  bool TimedWait(Mutex& mutex, uint64_t timeout) {
    std::unique_lock<std::mutex> lock(*mutex.InnerMutex());
    return (std::cv_status::no_timeout ==
        cond_.wait_for(lock, std::chrono::nanoseconds(timeout)));
  }
};

}

#else
# if defined(TYR_OS_WIN)
#  include "win/tyr_win_condition.h"
# elif defined(TYR_OS_LINUX)
#  include "posix/tyr_posix_condition.h"
# elif defined(TYR_OS_MAC)
#  include "mac/tyr_mac_condition.h"
#endif

#endif  // __TYR_CONDITION_HEADER_H__
