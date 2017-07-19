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
#include <functional>
#include <iostream>
#include <iterator>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Thread.h>
#include <Chaos/Memory/MemoryPool.h>
#include "object.h"
#include "parallel_memory.h"

namespace gc {

class Worker : private Chaos::UnCopyable {
  std::size_t id_{};
  bool running_{true};
  bool marking_{};
  mutable Chaos::Mutex mutex_;
  Chaos::Condition mark_cond_;
  Chaos::Thread thread_;
  std::vector<BaseObject*> roots_;
  std::vector<BaseObject*> worklist_;

  void mark(void) {
    // TODO:
  }

  void mark_from_roots(void) {
    for (auto* obj : roots_) {
      obj->set_marked();
      worklist_.push_back(obj);
      mark();
    }
  }

  void work_closure(void) {
    while (running_) {
      // TODO:
    }
  }
public:
  explicit Worker(std::size_t id)
    : id_(id)
    , mutex_()
    , mark_cond_(mutex_)
    , thread_(std::bind(&Worker::work_closure, this)) {
    thread_.start();
  }

  ~Worker(void) { stop(); thread_.join(); }
  void stop(void) { running_ = false; mark_cond_.notify_one(); }
  void marking(void) { marking_ = true; mark_cond_.notify_one(); }
  void put_in(BaseObject* obj) { roots_.push_back(obj); }
  BaseObject* fetch_out(void) {
    auto* obj = roots_.back();
    roots_.pop_back();
    return obj;
  }
};

ParallelMemory::ParallelMemory(void)
  : mutex_()
  , sweep_cond_(mutex_) {
  start_workers();
}

ParallelMemory::~ParallelMemory(void) {
  stop_workers();
}

void* ParallelMemory::alloc(std::size_t n) {
  return Chaos::MemoryPool::get_instance().alloc(n);
}

void ParallelMemory::dealloc(BaseObject* obj) {
  if (obj != nullptr)
    Chaos::MemoryPool::get_instance().dealloc(obj, obj->get_size());
}

void ParallelMemory::start_workers(void) {
  for (auto i = 0u; i < kWorkerNumber; ++i)
    workers_.emplace_back(new Worker(i));
}

void ParallelMemory::stop_workers(void) {
  for (auto& w: workers_)
    w->stop();
}

std::size_t ParallelMemory::put_in_order(void) {
  auto order = order_;
  order_ = (order_ + 1) % kWorkerNumber;
  return order;
}

std::size_t ParallelMemory::fetch_out_order(void) {
  order_ = (order_ - 1 + kWorkerNumber) % kWorkerNumber;
  return order_;
}

void ParallelMemory::sweep(void) {
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->is_marked()) {
      dealloc(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->unset_marked();
      ++it;
    }
  }
}

ParallelMemory& ParallelMemory::get_instance(void) {
  static ParallelMemory ins;
  return ins;
}

void ParallelMemory::notify_sweeping(std::size_t id) {
  {
    Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
    sweep_set_.insert(id);
  }
  sweep_cond_.notify_one();
}

void ParallelMemory::collect(void) {
  // TODO:
}

BaseObject* ParallelMemory::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new (alloc(sizeof(Int))) Int();
  obj->set_value(value);

  // TODO:
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelMemory::put_in(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new (alloc(sizeof(Pair))) Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  // TODO:
  objects_.push_back(obj);

  return nullptr;
}

BaseObject* ParallelMemory::fetch_out(void) {
  // TODO:
  return nullptr;
}

}
