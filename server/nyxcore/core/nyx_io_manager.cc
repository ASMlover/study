// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <functional>
#include <vector>
#include <Python.h>
#include "nyx_io_manager.h"

namespace nyx { namespace core {

static constexpr std::size_t kDefaultThreadNum = 3;

io_manager::io_manager(void)
  : thread_num_(kDefaultThreadNum) {
}

io_manager::~io_manager(void) {
  if (running_) {
    running_ = false;
    unset_work();

    if (thread_)
      thread_->join();
  }
}

void io_manager::thread_run(void) {
  std::vector<thread_ptr> threads;
  for (auto i = 0u; i < thread_num_; ++i) {
    auto t = std::make_shared<std::thread>([this] { io_service_.run(); });
    threads.push_back(t);
  }
  for (auto& t : threads)
    t->join();
}

void io_manager::start(void) {
  if (running_)
    return;

  running_ = true;
  set_work();
  thread_.reset(new std::thread([this] { thread_run(); }));
}

void io_manager::stop(void) {
  running_ = false;

  unset_work();
  io_service_.stop();

  Py_BEGIN_ALLOW_THREADS
  if (thread_)
    thread_->join();
  Py_END_ALLOW_THREADS
}

bool io_manager::poll(bool no_sleep) {
  if (running_) {
    // TODO:
  }

  return running_;
}

void io_manager::set_work(void) {
  work_.reset(new boost::asio::io_service::work(io_service_));
}

void io_manager::unset_work(void) {
  work_.reset();
}

}}
