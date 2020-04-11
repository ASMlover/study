// Copyright (c) 2020 ASMlover. All rights reserved.
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
#pragma once

#if defined(USE_ASIO_THREADPOOL)
# include <boost/asio.hpp>
#else
# include <deque>
#endif
#include "common.hh"

namespace coro {

using ThreadItem = std::unique_ptr<std::thread>;

#if defined(USE_ASIO_THREADPOOL)
class ThreadPool final : private UnCopyable {
  bool running_{true};
  std::vector<ThreadItem> threads_;
  boost::asio::io_context context_;
  boost::asio::io_context::work work_;
public:
  ThreadPool(int thread_num = 4) noexcept
    : work_(context_) {
    for (int i = 0; i < thread_num; ++i) {
      threads_.emplace_back(
          std::make_unique<std::thread>([this] { context_.run(); }));
    }
  }

  ~ThreadPool() noexcept {
    shutoff();
  }

  void shutoff() noexcept {
    if (running_) {
      running_ = false;
      context_.stop();

      for (auto& t : threads_)
        t->join();
    }
  }

  template <typename Fn, typename... Args>
  void post_task(Fn&& fn, Args&&... args) {
    context_.post(
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
  }
};
#else
class ThreadPool final : private UnCopyable {
  using TaskItem = std::function<void ()>;

  bool running_{true};
  mutable std::mutex m_;
  std::condition_variable non_empty_;
  std::vector<ThreadItem> threads_;
  std::deque<TaskItem> tasks_;

  TaskItem fetch_task() {
    std::unique_lock<std::mutex> lock(m_);

    while (running_ && tasks_.empty())
      non_empty_.wait(lock);

    TaskItem t;
    if (!tasks_.empty()) {
      t = std::move(tasks_.front());
      tasks_.pop_front();
    }
    return t;
  }
public:
  ThreadPool(int thread_num = 4) noexcept {
    for (int i = 0; i < thread_num; ++i) {
      threads_.emplace_back(std::make_unique<std::thread>([this] {
              while (running_) {
                auto t = fetch_task();
                if (t)
                  t();
              }
            }));
    }
  }

  ~ThreadPool() noexcept {
    shutoff();
  }

  void shutoff() noexcept {
    if (running_) {
      std::unique_lock<std::mutex> lock(m_);
      running_ = false;
      non_empty_.notify_all();
    }

    for (auto& t : threads_)
      t->join();
  }

  template <typename Fn, typename... Args>
  void post_task(Fn&& fn, Args&&... args) {
    using ReturnType = typename std::invoke_result<Fn, Args...>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType ()>>(
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));

    {
      std::unique_lock<std::mutex> lock(m_);
      tasks_.emplace_back([task] { (*task)(); });
    }
    non_empty_.notify_one();
  }
};
#endif

}
