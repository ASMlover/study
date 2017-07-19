// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <algorithm>
#include <iostream>
#include <functional>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <set>
#include <vector>

std::mutex g_outmutex;
std::set<int> g_finished;
std::condition_variable g_cv_finish;

class Worker {
  int id_{0};
  bool running_{true};
  std::mutex mutex_;
  std::condition_variable cv_work_;
  std::thread thread_;
  std::vector<int> datas_;

  Worker(const Worker&) = delete;
  Worker& operator=(const Worker&) = delete;

  void closure(void) {
    while (running_) {
      {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_work_.wait(lk, [this]{ return !datas_.empty() || !running_; });
      }
      if (!running_)
        break;

      auto sum = 0;
      std::for_each(datas_.begin(), datas_.end(), [&sum](int& v){ sum += v; });
      datas_.clear();

      {
        std::unique_lock<std::mutex> g(g_outmutex);
        std::cout << "Worker<" << id_ << "> sum is: " << sum << std::endl;
      }

      {
        std::unique_lock<std::mutex> lk(mutex_);
        g_finished.insert(id_);
      }
      g_cv_finish.notify_one();
    }
  }
public:
  Worker(int id)
    : id_(id)
    , thread_(std::bind(&Worker::closure, this)) {
  }

  void stop(void) {
    running_ = false;
    cv_work_.notify_one();
    thread_.join();
  }

  void put_in(int v) {
    datas_.push_back(v);
  }

  void start_work(void) {
    cv_work_.notify_one();
  }
};
using WorkerEntity = std::unique_ptr<Worker>;

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  constexpr int kWorkerNumber = 4;
  std::mutex mutex;

  std::vector<WorkerEntity> workers;
  for (auto i = 0; i < kWorkerNumber; ++i)
    workers.emplace_back(new Worker(i));

  std::this_thread::sleep_for(std::chrono::microseconds(100));

  g_finished.clear();
  auto order = 0;
  for (auto i = 0; i < 51200; ++i) {
    workers[order]->put_in(i);
    order = (order + 1) % kWorkerNumber;
  }
  for (auto& w: workers)
    w->start_work();

  {
    std::unique_lock<std::mutex> lk(mutex);
    g_cv_finish.wait(lk, []{ return g_finished.size() >= kWorkerNumber; });
  }

  for (auto& w : workers)
    w->stop();

  return 0;
}
