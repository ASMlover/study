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
#pragma once

#include <atomic>
#include <memory>
#include <list>
#include <vector>
#include <Chaos/UnCopyable.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Concurrent/Condition.h>

namespace gc {

class BaseObject;
class Worker;

class ParallelMark : private Chaos::UnCopyable {
  using WorkerEntity = std::unique_ptr<Worker>;

  int order_{};
  std::vector<WorkerEntity> workers_;
  std::list<BaseObject*> objects_;
  mutable Chaos::Mutex sweep_mutex_;
  Chaos::Condition sweep_cond_;
  std::atomic<int> sweep_counter_{};
  static constexpr int kWorkers = 4;
  static constexpr std::size_t kMaxObjects = 4096;

  friend class Worker;

  ParallelMark(void);
  ~ParallelMark(void);

  void start_workers(void);
  void stop_workers(void);
  int put_in_order(void);
  int fetch_out_order(void);
  void notify_sweeping(int worker_id);
  void sweep(void);
public:
  static ParallelMark& get_instance(void);

  void collect(void);
  BaseObject* put_in(int value);
  BaseObject* put_in(BaseObject* first = nullptr, BaseObject* second = nullptr);
  BaseObject* fetch_out(void);
};

}
