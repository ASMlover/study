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
#include <iterator>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Thread.h>
#include <Chaos/Memory/MemoryPool.h>
#include "object.h"
#include "parallel_sweep.h"

namespace gc {

class Sweeper : private Chaos::UnCopyable {
  int id_{};
  bool running_{true};
  bool tracing_{};
  mutable Chaos::Mutex mutex_;
  mutable Chaos::Mutex trace_mutex_;
  Chaos::Condition trace_cond_;
  Chaos::Thread thread_;
  std::vector<BaseObject*> roots_;
  std::vector<BaseObject*> marked_objects_;

  void acquire_work(void) {
    if (!marked_objects_.empty())
      return;

    {
      Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
      transfer(roots_.size() / 2, marked_objects_);
    }

    if (marked_objects_.empty())
      ParallelSweep::get_instance().acquire_work(id_, marked_objects_);
  }

  void perform_work(void) {
    while (!marked_objects_.empty()) {
      auto* obj = marked_objects_.back();
      marked_objects_.pop_back();

      obj->set_marked();
      if (obj->is_pair()) {
        auto append_fn = [this](BaseObject* o) {
          if (o != nullptr && !o->is_marked()) {
            o->set_marked(); marked_objects_.push_back(o);
          }
        };

        append_fn(Chaos::down_cast<Pair*>(obj)->first());
        append_fn(Chaos::down_cast<Pair*>(obj)->second());
      }
    }
  }

  void generate_work(void) {
    if (roots_.empty()) {
      Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
      std::copy(marked_objects_.begin(),
          marked_objects_.end(), std::back_inserter(roots_));
      marked_objects_.clear();
    }
  }

  void sweeper_closure(void) {
    while (running_) {
      {
        Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
        while (running_ && !tracing_)
          trace_cond_.wait();
        if (!running_)
          break;
      }

      while (tracing_) {
        acquire_work();
        perform_work();
        generate_work();

        if (marked_objects_.empty()) {
          tracing_ = false;
          ParallelSweep::get_instance().notify_sweeping(id_);
        }
      }
    }
  }
public:
  explicit Sweeper(int id)
    : id_(id)
    , trace_mutex_()
    , trace_cond_(trace_mutex_)
    , thread_([this] { sweeper_closure(); }) {
    thread_.start();
  }

  ~Sweeper(void) { stop(); thread_.join(); }
  int id(void) const { return id_; }
  void stop(void) { running_ = false; trace_cond_.notify_one(); }
  std::size_t object_count(void) const { return roots_.size(); }
  bool try_lock(void) { return mutex_.try_lock(); }
  void unlock(void) { mutex_.unlock(); }
  void tracing(void) { tracing_ = true; trace_cond_.notify_one(); }
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

ParallelSweep::ParallelSweep(void)
  : sweeper_mutex_()
  , sweeper_cond_(sweeper_mutex_) {
  startup_sweepers();
}

ParallelSweep::~ParallelSweep(void) {
  clearup_sweepers();
}

void ParallelSweep::startup_sweepers(void) {
  for (auto i = 0; i < kMaxSweepers; ++i)
    sweepers_.emplace_back(new Sweeper(i));
}

void ParallelSweep::clearup_sweepers(void) {
  for (auto& s : sweepers_)
    s->stop();
}

int ParallelSweep::put_in_order(void) {
  auto r = order_;
  order_ = (order_ + 1) % kMaxSweepers;
  return r;
}

int ParallelSweep::fetch_out_order(void) {
  order_ = (order_ - 1 + kMaxSweepers) % kMaxSweepers;
  return order_;
}

void* ParallelSweep::alloc(std::size_t n) {
  return Chaos::MemoryPool::get_instance().alloc(n);
}

void ParallelSweep::dealloc(BaseObject* obj) {
  Chaos::MemoryPool::get_instance().dealloc(obj);
}

void ParallelSweep::acquire_work(int id, std::vector<BaseObject*>& objects) {
  for (auto& s : sweepers_) {
    if (s->id() == id)
      continue;

    if (s->try_lock()) {
      s->transfer(s->object_count() / 2, objects);
      s->unlock();
      break;
    }
  }
}

void ParallelSweep::notify_sweeping(int /*id*/) {
  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweeper_mutex_);
    ++sweeper_counter_;
  }
  sweeper_cond_.notify_one();
}

void ParallelSweep::sweep(void) {
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

ParallelSweep& ParallelSweep::get_instance(void) {
  static ParallelSweep ins;
  return ins;
}

void ParallelSweep::collect(void) {
  auto old_count = objects_.size();

  sweeper_counter_ = 0;
  for (auto& s : sweepers_)
    s->tracing();

  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweeper_mutex_);
    while (sweeper_counter_ < kMaxSweepers)
      sweeper_cond_.wait();
  }

  sweep();

  std::cout
    << "[" << old_count - objects_.size() << "] objects collected, "
    << "[" << objects_.size() << "] objects remainning." << std::endl;
}

BaseObject* ParallelSweep::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new (alloc(sizeof(Int))) Int();
  obj->set_value(value);

  sweepers_[put_in_order()]->put_in(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelSweep::put_in(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new (alloc(sizeof(Pair))) Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  sweepers_[put_in_order()]->put_in(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelSweep::fetch_out(void) {
  return sweepers_[fetch_out_order()]->fetch_out();
}

}
