//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include "el_test_header.h"
#include "../el_thread_pool.h"



static inline void 
Worker(void* arg)
{
  char* name = static_cast<char*>(arg);

  for (int i = 0; i < 5; ++i) {
    fprintf(stdout, "\t\t[%s : %lu] - counter [%d]\n", 
        name, SelfThreadId(), i + 1);
    el_sleep(100);
  }
}



UNIT_IMPL(ThreadPool)
{
  el::ThreadPool thread_pool;

  thread_pool.Start();

  thread_pool.Run(Worker, (void*)"Worker_#1");
  thread_pool.Run(Worker, (void*)"Worker_#2");
  thread_pool.Run(Worker, (void*)"Worker_#3");

  thread_pool.Stop();
}




static bool s_running = false;
static void 
Worker2(void* arg)
{
  while (s_running) 
    el_sleep(100);

  fprintf(stdout, "\t\tWorker finished ...\n");
}

UNIT_IMPL(ThreadPool2)
{
  el::ThreadPool thread_pool;
  thread_pool.Start();

  s_running = true;
  thread_pool.Run(Worker2, NULL);

  el_sleep(1000);
  s_running = false;

  el_sleep(2000);
  thread_pool.Stop();
}
