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
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "TConfig.h"
#include "TException.h"
// #include "TLogging.h"
#include "TCurrentThread.h"
#include "unexposed/TCurrentThreadUnexposed.h"
#include "TThread.h"

namespace tyr { namespace basic {

void after_fork(void) {
  CurrentThread::unexposed::set_cached_tid(0);
  CurrentThread::unexposed::set_thread_name("main");
  CurrentThread::tid();
}

class ThreadNameInitialier {
public:
  ThreadNameInitialier(void) {
    CurrentThread::unexposed::set_thread_name("main");
    CurrentThread::tid();
    kern_thread_atfork(nullptr, nullptr, &after_fork);
  }
};
ThreadNameInitialier gInit;

struct ThreadData {
  ThreadCallback thrd_routine;
  std::string thrd_name;
  std::weak_ptr<pid_t> thrd_tid;

  ThreadData(const ThreadCallback& cb, const std::string& name, const std::shared_ptr<pid_t>& tid)
    : thrd_routine(cb)
    , thrd_name(name)
    , thrd_tid(tid) {
  }

  void run_in_thread(void) {
    pid_t tid = CurrentThread::tid();
    std::shared_ptr<pid_t> ptid = thrd_tid.lock();
    if (ptid) {
      *ptid = tid;
      ptid.reset();
    }

    CurrentThread::unexposed::set_thread_name(thrd_name.empty() ? "TyrThread" : thrd_name.c_str());
    kern_this_thread_setname(CurrentThread::name());
    try {
      thrd_routine();
      CurrentThread::unexposed::set_thread_name("finished");
    }
    catch (const Exception& ex) {
      CurrentThread::unexposed::set_thread_name("crashed");
      fprintf(stderr,
          "exception caught in Thread %s\n"
          "reason: %s\n"
          "stack trace: %s\n",
          thrd_name.c_str(),
          ex.what(),
          ex.stack_trace());
      abort();
    }
    catch (const std::exception& ex) {
      CurrentThread::unexposed::set_thread_name("crashed");
      fprintf(stderr,
          "exception caught in Thread %s\n"
          "reason: %s\n",
          thrd_name.c_str(),
          ex.what());
      abort();
    }
    catch (...) {
      CurrentThread::unexposed::set_thread_name("crashed");
      fprintf(stderr, "unknown exception caught in Thread %s\n", thrd_name.c_str());
      throw;
    }
  }
};

void* start_thread(void* argument) {
  ThreadData* data = static_cast<ThreadData*>(argument);
  data->run_in_thread();
  delete data;
  return nullptr;
}


std::atomic<int32_t> Thread::num_created_;

void Thread::set_default_name(void) {
  int32_t n = num_created_.fetch_add(1) + 1;
  if (name_.empty()) {
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "Thread%d", n);
    name_ = buf;
  }
}

Thread::Thread(ThreadCallback&& cb, const std::string& name)
  : started_(false)
  , joined_(false)
  , thread_(0)
  , tid_(new pid_t(0))
  , routine_(std::move(cb))
  , name_(name) {
  set_default_name();
}

Thread::~Thread(void) {
  if (started_ && !joined_)
    kern_thread_detach(thread_);
}

void Thread::start(void) {
  assert(!started_);
  started_ = true;

  ThreadData* data = new ThreadData(routine_, name_, tid_);
  if (0 != kern_thread_create(&thread_, &start_thread, data)) {
    started_ = false;
    delete data;
    // TL_SYSFATAL << "Failed in call pthread_create";
  }
}

int Thread::join(void) {
  assert(started_ && !joined_);
  joined_ = true;
  return kern_thread_join(thread_);
}

}}
