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
#include "../el_time.h"

static void Worker1(void* argument) {
  char* name = static_cast<char*>(argument);

  for (int i = 0; i < 5; ++i) {
    UNIT_PRINT("%s - counter(%d)\n", name, i + 1);
    el::Sleep(100);
  }
}

UNIT_IMPL(ThreadPool1) {
  el::ThreadPool pool;
  pool.Start();

  pool.Run(Worker1, (void*)"Worker_#1");
  pool.Run(Worker1, (void*)"Worker_#2");
  pool.Run(Worker1, (void*)"Worker_#3");

  pool.Stop();
}

static bool s_running = false;
static void Worker2(void* argument) {
  while (s_running)
    el::Sleep(100);

  UNIT_PRINT("Worker finished ...\n");
}

UNIT_IMPL(ThreadPool2) {
  el::ThreadPool pool;
  pool.Start();

  s_running = true;
  pool.Run(Worker2, nullptr);

  el::Sleep(1000);
  s_running = false;

  el::Sleep(2000);
  pool.Stop();
}
