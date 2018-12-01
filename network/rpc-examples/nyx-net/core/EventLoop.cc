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
#include <chrono>
#include <core/ServerManager.h>
#include <core/EventLoop.h>

namespace nyx {

static constexpr std::chrono::microseconds kPollTick(10000);

inline std::uint64_t now_as_microseconds(void) {
  return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
}

EventLoop::EventLoop(void)
  : thread_num_(kDefThreadNum) {
}

EventLoop::~EventLoop(void) {
  if (is_running_) {
    is_running_ = false;
    enable_worker(false);
    shutoff_threads();
  }
}

void EventLoop::launch_threads(void) {
  for (auto i = 0u; i < thread_num_; ++i)
    threads_.emplace_back(new std::thread([this] { context_.run(); }));
}

void EventLoop::shutoff_threads(void) {
  for (auto& t : threads_)
    t->join();
}

bool EventLoop::poll(void) {
  if (is_running_) {
    auto begin = now_as_microseconds();
    // TODO: call service callback bindings
    auto one_poll = now_as_microseconds() - begin;

    if (one_poll < 100)
      std::this_thread::sleep_for(kPollTick);
  }
  return is_running_;
}

void EventLoop::launch(void) {
  if (is_running_)
    return;

  is_running_ = true;
  enable_worker(true);
  launch_threads();
}

void EventLoop::shutoff(void) {
  is_running_ = false;
  enable_worker(false);
  context_.stop();
  shutoff_threads();
}

void EventLoop::enable_worker(bool enable) {
  if (enable)
    worker_.reset(new asio::io_context::work(context_));
  else
    worker_.reset();
}

}
