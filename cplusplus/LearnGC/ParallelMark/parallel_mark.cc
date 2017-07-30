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
#include <iostream>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Thread.h>
#include "object.h"
#include "parallel_mark.h"

namespace gc {

class Worker : private Chaos::UnCopyable {
  int id_{};
  bool running_{true};
  bool tracing_{};
  mutable Chaos::Mutex mutex_;
  Chaos::Condition trace_cond_;
  Chaos::Thread thread_;
  std::vector<BaseObject*> roots_;
  std::vector<BaseObject*> worklist_;

  void mark(void) {
    while (!worklist_.empty()) {
      auto* obj = worklist_.back();
      worklist_.pop_back();

      if (obj->is_pair()) {
        auto append_fn = [this](BaseObject* o) {
          if (o != nullptr && !o->is_marked()) {
            o->set_marked(); worklist_.push_back(o);
          }
        };

        append_fn(Chaos::down_cast<Pair*>(obj)->first());
        append_fn(Chaos::down_cast<Pair*>(obj)->second());
      }
    }
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
      {
        Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
        while (running_ && !tracing_)
          trace_cond_.wait();
        if (!running_)
          break;
      }

      mark_from_roots();

      if (tracing_) {
        tracing_ = false;
        ParallelMark::get_instance().notify_sweeping(id_);
      }
    }
  }
public:
  Worker(int id)
    : id_(id)
    , mutex_()
    , trace_cond_(mutex_)
    , thread_([this]{ work_closure(); }) {
    thread_.start();
  }

  ~Worker(void) { stop(); thread_.join(); }
  void stop(void) { running_ = false; trace_cond_.notify_one(); }
  void tracing(void) { tracing_ = true; trace_cond_.notify_one(); }
  void put_in(BaseObject* obj) { roots_.push_back(obj); }

  BaseObject* fetch_out(void) {
    auto* obj = roots_.back();
    roots_.pop_back();
    return obj;
  }
};

ParallelMark::ParallelMark(void)
  : sweep_mutex_()
  , sweep_cond_(sweep_mutex_) {
  start_workers();
}

ParallelMark::~ParallelMark(void) {
  stop_workers();
}

void ParallelMark::start_workers(void) {
  for (int i = 0; i < kWorkers; ++i)
    workers_.emplace_back(new Worker(i));
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

void ParallelMark::notify_sweeping(int /*id*/) {
  ++sweep_counter_;
  sweep_cond_.notify_one();
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
  auto old_count = objects_.size();

  sweep_counter_ = 0;
  for (auto& w : workers_)
    w->tracing();

  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweep_mutex_);
    while (sweep_counter_ < kWorkers)
      sweep_cond_.wait();
  }

  sweep();

  std::cout
    << "[" << old_count - objects_.size() << "] objects collected, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
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
