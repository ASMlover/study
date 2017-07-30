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
#include <list>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Thread.h>
#include <Chaos/Memory/MemoryPool.h>
#include "object.h"
#include "parallel_channel.h"

namespace gc {

class Sweeper : private Chaos::UnCopyable {
  int id_{};
  bool running_{true};
  bool tracing_{};
  bool sweeping_{};
  mutable Chaos::Mutex trace_mutex_;
  Chaos::Condition trace_cond_;
  mutable Chaos::Mutex sweep_mutex_;
  Chaos::Condition sweep_cond_;
  Chaos::Thread thread_;
  std::vector<BaseObject*> marked_objects_;
  std::list<BaseObject*> objects_;

  void* alloc(std::size_t n) {
    return Chaos::MemoryPool::get_instance().alloc(n);
  }

  void dealloc(BaseObject* obj) {
    if (obj != nullptr)
      Chaos::MemoryPool::get_instance().dealloc(obj, obj->get_size());
  }

  void perform_work(void) {
    // TODO:
  }

  void sweep(void) {
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

  void sweeper_closure(void) {
    while (running_) {
      {
        Chaos::ScopedLock<Chaos::Mutex> g(trace_mutex_);
        while (running_ && !tracing_)
          trace_cond_.wait();
        if (!running_)
          break;
      }

      marked_objects_.clear();
      ParallelChannel::get_instance().init_sweeper_marked_objects(
          id_, marked_objects_);

      // TODO:
      ParallelChannel::get_instance().notify_sweeping(id_);

      {
        Chaos::ScopedLock<Chaos::Mutex> g(sweep_mutex_);
        while (running_ && !sweeping_)
          sweep_cond_.wait();
        if (!running_)
          break;
      }

      if (sweeping_) {
        sweep();
        sweeping_ = false;
        ParallelChannel::get_instance().notify_sweeped(id_, objects_.size());
      }
    }
  }
public:
  explicit Sweeper(int id)
    : id_(id)
    , trace_mutex_()
    , trace_cond_(trace_mutex_)
    , sweep_mutex_()
    , sweep_cond_(sweep_mutex_)
    , thread_([this]{ sweeper_closure(); }) {
    thread_.start();
  }

  ~Sweeper(void) {
    stop();
    thread_.join();
  }

  void stop(void) {
    running_ = false;
    trace_cond_.notify_one();
    sweep_cond_.notify_one();
  }

  void put_in(BaseObject* obj) { objects_.push_back(obj); }
  void tracing(void) { tracing_ = true; trace_cond_.notify_one(); }
  void sweeping(void) { sweeping_ = true; sweep_cond_.notify_one(); }
};

ParallelChannel::ParallelChannel(void)
  : sweeper_mutex_()
  , sweeper_cond_(sweeper_mutex_) {
  startup_sweepers();
}

ParallelChannel::~ParallelChannel(void) {
  clearup_sweepers();
}

void ParallelChannel::startup_sweepers(void) {
  for (auto i = 0; i < kMaxSweepers; ++i)
    sweepers_.emplace_back(new Sweeper(i));
}

void ParallelChannel::clearup_sweepers(void) {
  for (auto& s : sweepers_)
    s->stop();
}


int ParallelChannel::put_in_order(void) {
  int r = order_;
  order_ = (order_ + 1) % kMaxSweepers;
  return r;
}

int ParallelChannel::fetch_out_order(void) {
  order_ = (order_ - 1 + kMaxSweepers) % kMaxSweepers;
  return order_;
}

void ParallelChannel::put_into_sweeper(BaseObject* obj) {
  // TODO:
}

void ParallelChannel::init_sweeper_marked_objects(
    int sweeper_id, std::vector<BaseObject*>& objects) {
  // TODO:
}

void ParallelChannel::acquire_work(
    int sweeper_id, std::vector<BaseObject*>& objects) {
  // TODO:
}

bool ParallelChannel::generate_work(int sweeper_id, BaseObject* obj) {
  return false;
}

void ParallelChannel::notify_sweeping(int sweeper_id) {
  // TODO:
}

void ParallelChannel::notify_sweeped(int sweeper_id, std::size_t alive_count) {
  // TODO:
}

ParallelChannel& ParallelChannel::get_instance(void) {
  static ParallelChannel ins;
  return ins;
}

void ParallelChannel::collect(void) {
  // TODO:
}

BaseObject* ParallelChannel::put_in(int value) {
  // TODO:
  return nullptr;
}

BaseObject* ParallelChannel::put_in(BaseObject* first, BaseObject* second) {
  // TODO:
  return nullptr;
}

BaseObject* ParallelChannel::fetch_out(void) {
  // TODO:
  return nullptr;
}

}
