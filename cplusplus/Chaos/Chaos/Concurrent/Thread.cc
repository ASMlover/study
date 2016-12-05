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
#include <stdio.h>
#include <stdlib.h>
#include <Chaos/Except/Exception.h>
#include <Chaos/IO/ColorIO.h>
#include <Chaos/Logging/Logging.h>
#include <Chaos/Concurrent/Unexposed/CurrentThread.h>
#include <Chaos/Concurrent/CurrentThread.h>
#include <Chaos/Concurrent/Thread.h>

namespace Chaos {

static void after_fork(void) {
  CurrentThread::Unexposed::set_cached_tid(0);
  CurrentThread::Unexposed::set_name("main");
  CurrentThread::get_tid();
}

class _ThreadNameInitializer {
public:
  _ThreadNameInitializer(void) {
    CurrentThread::Unexposed::set_name("main");
    CurrentThread::get_tid();
    Chaos::kern_thread_atfork(nullptr, nullptr, &after_fork);
  }
};
_ThreadNameInitializer g_threadname_init;

struct ThreadData {
  ThreadCallback _fn;
  std::string _name;
  std::weak_ptr<pid_t> _wk_tid;

  ThreadData(const ThreadCallback& fn, const std::string& name, const std::shared_ptr<pid_t>& tid)
    : _fn(fn)
    , _name(name)
    , _wk_tid(tid) {
  }

  void run_in_thread(void) {
    pid_t tid = CurrentThread::get_tid();
    std::shared_ptr<pid_t> tidp = _wk_tid.lock();
    if (tidp) {
      *tidp = tid;
      tidp.reset();
    }

    CurrentThread::Unexposed::set_name(_name.empty() ? "ChaosThread" : _name.c_str());
    Chaos::kern_this_thread_setname(CurrentThread::get_name());
    try {
      _fn();
      CurrentThread::Unexposed::set_name("finished");
    }
    catch (const Exception& ex) {
      CurrentThread::Unexposed::set_name("crashed");
      ColorIO::fprintf(stderr,
          ColorIO::ColorType::COLORTYPE_RED,
          "exception caught in Chaos.Thread %s\n"
          "reason: %s\n"
          "stack traceback: %s\n",
          _name.c_str(),
          ex.what(),
          ex.get_traceback());
      abort();
    }
    catch (const std::exception& ex) {
      CurrentThread::Unexposed::set_name("crashed");
      ColorIO::fprintf(stderr,
          ColorIO::ColorType::COLORTYPE_RED,
          "exception caught in Chaos.Thread %s\n"
          "reason: %s\n",
          _name.c_str(),
          ex.what());
      abort();
    }
    catch (...) {
      CurrentThread::Unexposed::set_name("crashed");
      ColorIO::fprintf(stderr,
          ColorIO::ColorType::COLORTYPE_RED,
          "unknown exception caught in Chaos.Thread %s\n",
          _name.c_str());
      throw;
    }
  }
};

static void* start_thread(void* arg) {
  std::unique_ptr<ThreadData> td(static_cast<ThreadData*>(arg));
  if (td)
    td->run_in_thread();
  return nullptr;
}

std::atomic<int32_t> Thread::num_created_;

void Thread::set_default_name(void) {
  int32_t n = ++num_created_;
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Thread%d", n);
    name_ = buf;
  }
}

Thread::Thread(const ThreadCallback& fn, const std::string& name)
  : tid_(new pid_t(0))
  , routine_(fn)
  , name_(name) {
  set_default_name();
}

Thread::Thread(ThreadCallback&& fn, const std::string& name)
  : tid_(new pid_t(0))
  , routine_(std::move(fn))
  , name_(name) {
  set_default_name();
}

void Thread::start(void) {
  CHAOS_CHECK(!started_, "Thread::start - thread should be not started yet");

  started_ = true;
  std::unique_ptr<ThreadData> td(new ThreadData(routine_, name_, tid_));
  if (0 == Chaos::kern_thread_create(&thread_, &start_thread, td.get())) {
    td.release();
  }
  else {
    started_ = false;
    CHAOSLOG_SYSFATAL << "failed in call `Chaos::kern_thread_create`";
  }
}

bool Thread::join(void) {
  CHAOS_CHECK(started_ && !joined_, "Thread::join - thread should started and not joined");

  joined_ = true;
  return 0 == Chaos::kern_thread_join(thread_);
}

}
