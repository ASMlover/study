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
#include <chrono>
#include <functional>
#include <iostream>
#include <iterator>
#include <set>
#include <mutex>
#include <thread>
#include <Chaos/Types.h>
#include "object.h"
#include "parallel_sweep.h"

namespace gc {

class Worker {
  int order_{};
  bool stop_{};
  bool run_tracing_{};
  std::mutex mutex_;
  std::unique_ptr<std::thread> thread_;
  std::vector<BaseObject*> roots_;
  std::vector<BaseObject*> mark_objects_;

  friend class ParallelSweep;

  void acquire_work(void) {
    if (!mark_objects_.empty())
      return;

    {
      std::unique_lock<std::mutex> g(mutex_);
      transfer(roots_.size() / 2, mark_objects_);
    }

    if (mark_objects_.empty())
      ParallelSweep::get_instance().acquire_work(order_, mark_objects_);
  }

  void perform_work(void) {
    while (!mark_objects_.empty()) {
      auto* obj = mark_objects_.back();
      mark_objects_.pop_back();

      obj->set_marked();
      if (obj->is_pair()) {
        auto append_fn = [this](BaseObject* o) {
          if (o != nullptr && !o->is_marked()) {
            o->set_marked(); mark_objects_.push_back(o);
          }
        };

        append_fn(Chaos::down_cast<Pair*>(obj)->first());
        append_fn(Chaos::down_cast<Pair*>(obj)->second());
      }
    }
  }

  void generate_work(void) {
    if (roots_.empty()) {
      std::unique_lock<std::mutex> g(mutex_);
      std::copy(mark_objects_.begin(),
          mark_objects_.end(), std::back_inserter(roots_));
      mark_objects_.clear();
    }
  }

  void worker_routine(void) {
    while (!stop_) {
      if (run_tracing_) {
        acquire_work();
        perform_work();
        generate_work();
      }
      if (run_tracing_ && mark_objects_.empty())
        run_tracing_ = false;

      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  }
public:
  Worker(int order)
    : order_(order)
    , thread_(new std::thread(std::bind(&Worker::worker_routine, this)))
  {}
  ~Worker(void) { stop(); thread_->join(); }
  void stop(void) { stop_ = true; }
  void run_tracing(void) { run_tracing_ = true; }
  bool is_tracing(void) const { return run_tracing_; }
  std::size_t roots_count(void) const { return roots_.size(); }
  bool try_lock(void) { return mutex_.try_lock(); }
  void unlock(void) { mutex_.unlock(); }
  void put_in(BaseObject* obj) { roots_.push_back(obj); }

  BaseObject* fetch_out(void) {
    if (!roots_.empty()) {
      auto* obj = roots_.back();
      roots_.pop_back();
      return obj;
    }
    return nullptr;
  }

  void transfer(std::size_t n, std::vector<BaseObject*>& objects) {
    std::copy_n(roots_.begin(), n, std::back_inserter(objects));
    roots_.erase(roots_.begin(), roots_.begin() + n);
  }
};

ParallelSweep::ParallelSweep(void) {
  start_workers();
}

ParallelSweep::~ParallelSweep(void) {
  stop_workers();
}

void ParallelSweep::start_workers(int nworkers) {
  nworkers_ = nworkers;
  for (auto i = 0; i < nworkers_; ++i)
    workers_.emplace_back(new Worker(i));
}

void ParallelSweep::stop_workers(void) {
  for (auto i = 0; i < nworkers_; ++i)
    workers_[i]->stop();
}

int ParallelSweep::put_in_order(void) {
  int r = order_;
  order_ = (order_ + 1) % nworkers_;
  return r;
}

int ParallelSweep::fetch_out_order(void) {
  order_ = (order_ - 1 + nworkers_) % nworkers_;
  return order_;
}

void ParallelSweep::sweep(void) {
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

ParallelSweep& ParallelSweep::get_instance(void) {
  static ParallelSweep ins;
  return ins;
}

void ParallelSweep::acquire_work(
    int own_order, std::vector<BaseObject*>& objects) {
  for (auto i = 0; i < nworkers_; ++i) {
    if (i == own_order)
      continue;

    if (workers_[i]->try_lock()) {
      workers_[i]->transfer(workers_[i]->roots_count() / 2, objects);
      workers_[i]->unlock();
      break;
    }
  }
}

void ParallelSweep::collect(void) {
  auto old_count = objects_.size();

  for (auto i = 0; i < nworkers_; ++i)
    workers_[i]->run_tracing();

  std::set<int> finished;
  while (true) {
    for (auto i = 0; i < nworkers_; ++i) {
      if (!workers_[i]->is_tracing())
        finished.insert(i);
    }

    if (finished.size() == static_cast<std::size_t>(nworkers_))
      break;
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
  finished.clear();

  sweep();

  std::cout
    << "[" << old_count - objects_.size() << "] objects collected, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

BaseObject* ParallelSweep::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new Int();
  obj->set_value(value);

  auto order = put_in_order();
  workers_[order]->put_in(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelSweep::put_in(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  auto order = put_in_order();
  workers_[order]->put_in(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelSweep::fetch_out(void) {
  auto order = fetch_out_order();
  return workers_[order]->fetch_out();
}

}
