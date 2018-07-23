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
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <mutex>
#include <thread>
#include "nyx_utils.h"
#include "nyx_task_helper.h"

namespace nyx {

std::mutex _m;
std::atomic<bool> _need_notify{};
std::atomic<bool> _use_condition{};
std::condition_variable _cv;
std::chrono::microseconds _time_per_tick{kTimePerTick};

void task_wait(std::int64_t time) {
  if (time == 0)
    return;

  if (_use_condition) {
    std::unique_lock<std::mutex> guard(_m);
    _need_notify = true;
    if (time > 0 && time < kTimePerTick)
      _cv.wait_for(guard, std::chrono::microseconds(time));
    else
      _cv.wait_for(guard, _time_per_tick);
    _need_notify = false;
  }
  else {
    if (time > 0 && time < kTimePerTick)
      std::this_thread::sleep_for(std::chrono::microseconds(time));
    else
      std::this_thread::sleep_for(_time_per_tick);
  }
}

void task_notify(void) {
  if (_use_condition && _need_notify) {
    _m.lock();
    if (_need_notify) {
      _need_notify = false;
      _m.unlock();
      _cv.notify_one();
    }
    else {
      _m.unlock();
    }
  }
}

}
