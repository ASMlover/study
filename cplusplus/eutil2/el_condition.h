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
#ifndef __EL_CONDITION_HEADER_H__
#define __EL_CONDITION_HEADER_H__

#if defined(EUTIL_WIN)
  typedef struct {
    uint32_t         waiters_count;
    CRITICAL_SECTION waiters_count_lock;
    HANDLE           single_event;
    HANDLE           broadcast_event;
  } CondVar;
#elif defined(EUTIL_LINUX)
  typedef pthread_cond_t CondVar;
#endif

namespace el {

class Condition : private UnCopyable {
  Mutex&  mutex_;
  CondVar cond_;
public:
  explicit Condition(Mutex& mutex);
  ~Condition(void);

  void Signal(void);
  void SignalAll(void);
  void Wait(void);
  void TimedWait(uint64_t timeout);
};

}

#endif  // __EL_CONDITION_HEADER_H__
