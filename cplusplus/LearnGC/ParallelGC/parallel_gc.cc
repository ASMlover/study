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
#include <list>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Thread.h>
#include "object.h"
#include "parallel_gc.h"

namespace gc {

class Sweeper : private Chaos::UnCopyable {
  int id_{};
  bool running_{true};
  bool sweeping_{};
  mutable Chaos::Mutex mutex_;
  Chaos::Condition cond_;
  Chaos::Thread thread_;
  std::vector<BaseObject*> roots_;
  std::list<BaseObject*> objects_;
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

  void sweep(void) {
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

  void sweeper_closure(void) {
    while (running_) {
      {
        Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
        while (running_ && !sweeping_)
          cond_.wait();
        if (!running_)
          break;
      }

      mark_from_roots();
      sweep();

      if (sweeping_) {
        sweeping_ = false;
        ParallelGC::get_instance().notify_collected(id_, objects_.size());
      }
    }
  }
public:
  Sweeper(int id)
    : id_(id)
    , mutex_()
    , cond_(mutex_)
    , thread_([this]{ sweeper_closure(); }) {
    thread_.start();
  }

  ~Sweeper(void) { stop(); thread_.join(); }
  void stop(void) { running_ = false; cond_.notify_one(); }
  void collect(void) { sweeping_ = true; cond_.notify_one(); }

  void put_in(BaseObject* obj) {
    roots_.push_back(obj);
    objects_.push_back(obj);
  }

  BaseObject* fetch_out(void) {
    auto* obj = roots_.back();
    roots_.pop_back();
    return obj;
  }
};

ParallelGC::ParallelGC(void)
  : sweeper_mutex_()
  , sweeper_cond_(sweeper_mutex_) {
  startup_sweepers();
}

ParallelGC::~ParallelGC(void) {
  clearup_sweepers();
}

void ParallelGC::startup_sweepers(void) {
  for (auto i = 0u; i < kMaxSweepers; ++i)
    sweepers_.emplace_back(new Sweeper(i));
}

void ParallelGC::clearup_sweepers(void) {
  for (auto& s : sweepers_)
    s->stop();
}

int ParallelGC::put_in_order(void) {
  int r = order_;
  order_ = (order_ + 1) % kMaxSweepers;
  return r;
}

int ParallelGC::fetch_out_order(void) {
  order_ = (order_ - 1 + kMaxSweepers) % kMaxSweepers;
  return order_;
}

void ParallelGC::notify_collected(int /*id*/, std::size_t remain_count) {
  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweeper_mutex_);
    object_counter_ += remain_count;
  }
  ++sweeper_counter_;
  sweeper_cond_.notify_one();
}

ParallelGC& ParallelGC::get_instance(void) {
  static ParallelGC ins;
  return ins;
}

void ParallelGC::collect(void) {
  auto old_count = object_counter_;

  sweeper_counter_ = 0;
  object_counter_ = 0;
  for (auto& s : sweepers_)
    s->collect();

  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweeper_mutex_);
    while (sweeper_counter_ < kMaxSweepers)
      sweeper_cond_.wait();
  }

  std::cout
    << "[" << old_count - object_counter_ << "] objects collected, "
    << "[" << object_counter_ << "] objects remaining." << std::endl;
}

BaseObject* ParallelGC::put_in(int value) {
  if (object_counter_ >= kMaxObjects)
    collect();

  auto* obj = new Int();
  obj->set_value(value);

  ++object_counter_;
  sweepers_[put_in_order()]->put_in(obj);

  return obj;
}

BaseObject* ParallelGC::put_in(BaseObject* first, BaseObject* second) {
  if (object_counter_ >= kMaxObjects)
    collect();

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  ++object_counter_;
  sweepers_[put_in_order()]->put_in(obj);

  return obj;
}

BaseObject* ParallelGC::fetch_out(void) {
  return sweepers_[fetch_out_order()]->fetch_out();
}

}
