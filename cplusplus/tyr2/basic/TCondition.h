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
#ifndef __TYR_BASIC_CONDITION_HEADER_H__
#define __TYR_BASIC_CONDITION_HEADER_H__

#include "TPlatform.h"
#include "TMutex.h"

namespace tyr { namespace basic {

class Condition : private UnCopyable {
  Mutex& mtx_;
  KernCond cond_;
public:
  explicit Condition(Mutex& mtx)
    : mtx_(mtx) {
    kern_cond_init(&cond_);
  }

  ~Condition(void) {
    kern_cond_destroy(&cond_);
  }

  void wait(void) {
    Mutex::UnassignedGuard guard(mtx_);
    kern_cond_wait(&cond_, mtx_.get_mutex());
  }

  bool timed_wait(int seconds) {
    Mutex::UnassignedGuard guard(mtx_);
    return 0 == kern_cond_timedwait(&cond_, mtx_.get_mutex(), seconds * NANOSEC);
  }

  void signal(void) {
    kern_cond_signal(&cond_);
  }

  void broadcast(void) {
    kern_cond_broadcast(&cond_);
  }
};

}}

#endif // __TYR_BASIC_CONDITION_HEADER_H__
