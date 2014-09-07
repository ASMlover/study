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
#include "el_test.h"

static int el_count1 = 0;
static int el_count2 = 0;
static const int kCount = 5;

static void RoutineMutex(void* arg) {
  el::Mutex* mutex = static_cast<el::Mutex*>(arg);

  for (int i = 0; i < 1000; ++i) {
    el::LockerGuard<el::Mutex> guard(*mutex);

    ++el_count1;
    ++el_count2;
  }
}

static void RoutineSpinLock(void* arg) {
  el::SpinLock* spinlock = static_cast<el::SpinLock*>(arg);

  for (int i = 0; i < 1000; ++i) {
    el::LockerGuard<el::SpinLock> guard(*spinlock);

    ++el_count1;
    ++el_count2;
  }
}

UNIT_IMPL(Mutex) {
  std::vector<ThreadPtr> threads;
  el::Mutex mutex;

  el_count1 = 0;
  el_count2 = 0;
  for (int i = 0; i < kCount; ++i) {
    threads.push_back(ThreadPtr(new el::Thread()));
    threads[i]->Create(EL_THREAD_FUNCALL(RoutineMutex), &mutex);
  }
  threads.clear();

  UNIT_PRINT("el_count1 = %d\n", el_count1);
  UNIT_PRINT("el_count2 = %d\n", el_count2);
}

UNIT_IMPL(SpinLock) {
  std::vector<ThreadPtr> threads;
  el::SpinLock spinlock;

  el_count1 = 0;
  el_count2 = 0;
  for (int i = 0; i < kCount; ++i) {
    threads.push_back(ThreadPtr(new el::Thread()));
    threads[i]->Create(EL_THREAD_FUNCALL(RoutineSpinLock), &spinlock);
  }
  threads.clear();

  UNIT_PRINT("el_count1 = %d\n", el_count1);
  UNIT_PRINT("el_count2 = %d\n", el_count2);
}
