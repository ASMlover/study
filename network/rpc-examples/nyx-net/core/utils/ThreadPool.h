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
#pragma once

#include <functional>
#include <thread>
#include <vector>
#include <core/NyxInternal.h>

namespace nyx::utils {

class ThreadPool : private UnCopyable {
  using ThreadPtr = std::unique_ptr<std::thread>;

  bool stopped_{};
  int thread_num_{};
  std::vector<ThreadPtr> threads_;
  asio::io_context context_;
  asio::io_context::work work_;

  static constexpr int kDefThreadNum = 4;
public:
  ThreadPool(int thread_num = kDefThreadNum)
    : thread_num_(thread_num)
    , work_(context_) {
    for (auto i = 0; i < thread_num_; ++i)
      threads_.emplace_back(new std::thread([this] { context_.run(); }));
  }

  ~ThreadPool(void) {
    stop();
  }

  void shutoff(void) {
    if (!stopped_) {
      context_.stop();
      for (auto& t : threads_)
        t->join();
      stopped_ = true;
    }
  }

  template <typename Function, typename... Args>
  void post(Function&& fn, Args&&... args) {
    context_.post(
        std::bind(std::forward<Function>(fn), std::forward<Args>(args)...));
  }
};

}
