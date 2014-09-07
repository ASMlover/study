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

typedef std::shared_ptr<el::Thread> ThreadPtr;

static void FunctionRoutine(void* argument) {
  for (int i = 0; i < 10; ++i)
    UNIT_PRINT("%s -> Number Index : %d\n", __func__, i);
}

UNIT_IMPL(ThreadFunction) {
  ThreadPtr thread(new el::Thread());

  thread->Create(EL_THREAD_FUNCALL(FunctionRoutine));
}

class ClassRoutine : private el::UnCopyable {
  ThreadPtr thread_;
public:
  ClassRoutine(void) 
    : thread_(new el::Thread()) {
  }

  void Start(void) {
    thread_->Create(EL_THREAD_CLSCALL(ClassRoutine::Routine, this));
  }

  void Routine(void* argument) {
    for (int i = 0; i < 10; i++)
      UNIT_PRINT("%s -> Number Index : %d\n", __func__, i);
  }
};

UNIT_IMPL(ThreadClass) {
  ClassRoutine cr;

  cr.Start();
}
