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
#ifndef __TYR_BASIC_MUTEX_HEADER_H__
#define __TYR_BASIC_MUTEX_HEADER_H__

#include <assert.h>
#include "TTypes.h"
#include "TPlatform.h"
#include "TCurrentThread.h"

namespace tyr { namespace basic {

class Mutex : private UnCopyable {
  kern_mutex_t mtx_;
  pid_t holder_;
public:
  Mutex(void)
    : holder_(0) {
    kern_mutex_init(&mtx_);
  }

  ~Mutex(void) {
    assert(0 == holder_);
    kern_mutex_destroy(&mtx_);
  }

  bool locked_by_this_thread(void) const {
    return holder_ == CurrentThread::tid();
  }

  void assert_locked(void) const {
    assert(locked_by_this_thread());
  }

  void lock(void) {
    kern_mutex_lock(&mtx_);
    assign_holder();
  }

  void unlock(void) {
    unassign_holder();
    kern_mutex_unlock(&mtx_);
  }

  kern_mutex_t* get_mutex(void) {
    return &mtx_;
  }
private:
  void assign_holder(void) {
    holder_ = CurrentThread::tid();
  }

  void unassign_holder(void) {
    holder_ = 0;
  }
private:
  friend class Condition;

  class UnassignedGuard : private UnCopyable {
    Mutex& owner_;
  public:
    explicit UnassignedGuard(Mutex& owner)
      : owner_(owner) {
      owner_.unassign_holder();
    }

    ~UnassignedGuard(void) {
      owner_.assign_holder();
    }
  };
};

class MutexGuard : private UnCopyable {
  Mutex& mtx_;
public:
  MutexGuard(Mutex& mtx)
    : mtx_(mtx) {
    mtx_.lock();
  }

  ~MutexGuard(void) {
    mtx_.unlock();
  }
};

#define MutexGuard(mtx) error "missing guard object name"

}}

#endif // __TYR_BASIC_MUTEX_HEADER_H__
