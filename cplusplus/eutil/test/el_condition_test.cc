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
#include "el_unit.h"
#include "../el_condition.h"


class Worker : private el::UnCopyable {
  el::Mutex     mutex_;
  el::Condition cond_;
  bool          use_signal_all_;
  volatile bool posted_;
public:
  Worker(void) 
    : mutex_()
    , cond_(mutex_)
    , use_signal_all_(false) 
    , posted_(false) {
  }

  ~Worker(void) {
  }

  inline void RunWorker(void* arg) {
    mutex_.Lock();
    EL_ASSERT(!posted_);
    if (use_signal_all_)
      cond_.SignalAll();
    else 
      cond_.Signal();
    posted_ = true;
    mutex_.Unlock();
  }

  void Signal(void) {
    el::Thread thread;
    thread.Create(EL_THREAD_CALLBACK(Worker::RunWorker, this));

    mutex_.Lock();
    cond_.Wait();
    EL_ASSERT(posted_);
    mutex_.Unlock();
  }

  void SignalAll(void) {
    use_signal_all_ = true;

    el::Thread thread;
    thread.Create(EL_THREAD_CALLBACK(Worker::RunWorker, this));

    mutex_.Lock();
    cond_.Wait();
    EL_ASSERT(posted_);
    mutex_.Unlock();
  }
};



UNIT_IMPL(ConditionSignal) {
  Worker w;

  w.Signal();
}

UNIT_IMPL(ConditionSignalAll) {
  Worker w;

  w.SignalAll();
}
