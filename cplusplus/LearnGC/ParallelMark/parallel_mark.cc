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
#include <functional>
#include <iostream>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Thread.h>
#include "object.h"
#include "parallel_mark.h"

namespace gc {

class Worker : private Chaos::UnCopyable {
  int id_{};
  bool stop_{};
  Chaos::Thread thread_;
  std::vector<BaseObject*> roots_;

  void work_closure(void) {
    while (!stop_) {
    }
  }
public:
  Worker(int id)
    : id_(id)
    , thread_(std::bind(&Worker::work_closure, this))
  {
  }
  ~Worker(void) {}
  int get_id(void) const { return id_; }
  void start(void) { thread_.start(); }
  void stop(void) { stop_ = true; }
  void put_in(BaseObject* obj) { roots_.push_back(obj); }
  BaseObject* fetch_out(void) {
    auto* obj = roots_.back();
    roots_.pop_back();
    return obj;
  }
};

ParallelMark::ParallelMark(void) { start_workers(); }
ParallelMark::~ParallelMark(void) { stop_workers(); }

void ParallelMark::start_workers(void) {
  for (auto i = 0; i < kWorkers; ++i) {
    workers_.emplace_back(new Worker(i));
    workers_[i]->start();
  }
}

void ParallelMark::stop_workers(void) {
  for (auto& w : workers_)
    w->stop();
}

int ParallelMark::put_in_order(void) {
  auto order = order_;
  order_ = (order_ + 1) % kWorkers;
  return order;
}

int ParallelMark::fetch_out_order(void) {
  order_ = (order_ - 1 + kWorkers) % kWorkers;
  return order_;
}

void ParallelMark::sweep(void) {
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->is_marked()) {
      delete *it;
      objects_.erase(it++);
    }
    else {
      (*it)->unset_marked();
      ++it;
    }
  }
}

ParallelMark& ParallelMark::get_instance(void) {
  static ParallelMark ins;
  return ins;
}

void ParallelMark::collect(void) {
  // TODO:
}

BaseObject* ParallelMark::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new Int();
  obj->set_value(value);

  workers_[put_in_order()]->put_in(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelMark::put_in(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  workers_[put_in_order()]->put_in(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelMark::fetch_out(void) {
  return workers_[fetch_out_order()]->fetch_out();
}

}
