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
#include "../el_unit.h"
#include "../el_time.h"

class CondWorker : private el::UnCopyable {
  typedef std::shared_ptr<el::Thread> ThreadPtr;
  el::Mutex     mutex_;
  el::Condition cond_;
  bool          signal_all_;
  volatile bool posted_;
  ThreadPtr     thread_;
public:
  CondWorker(void)
    : mutex_()
    , cond_(mutex_)
    , signal_all_(false) 
    , posted_(false)
    , thread_(ThreadPtr(new el::Thread())) {
  }

  ~CondWorker(void) {
  }

  inline void Run(void* arg) {
    el::Sleep(100);
    el::LockerGuard<el::Mutex> guard(mutex_);
    
    EL_ASSERT(!posted_);
    if (signal_all_)
      cond_.SignalAll();
    else
      cond_.Signal();
    posted_ = true;
  }

  inline void Singal(void) {
    thread_->Create(EL_THREAD_CLSCALL(CondWorker::Run, this));

    el::LockerGuard<el::Mutex> guard(mutex_);
    cond_.Wait();
    EL_ASSERT(posted_);
  }

  inline void SignalAll(void) {
    signal_all_ = true;
    thread_->Create(EL_THREAD_CLSCALL(CondWorker::Run, this));

    el::LockerGuard<el::Mutex> guard(mutex_);
    cond_.Wait();
    EL_ASSERT(posted_);
  }
};


UNIT_IMPL(ConditionSignal) {
  CondWorker cw;
  cw.Singal();
}

UNIT_IMPL(ConditionSignalAll) {
  CondWorker cw;
  cw.SignalAll();
}
