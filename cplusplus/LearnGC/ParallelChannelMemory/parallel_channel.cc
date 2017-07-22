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
  bool sweeping_{};
  mutable Chaos::Mutex mutex_;
  Chaos::Condition cond_;
  Chaos::Thread thread_;
  std::vector<BaseObject*> marked_objects_;
  std::list<BaseObject*> objects_;

  void dealloc(BaseObject* obj) {
    if (obj != nullptr)
      Chaos::MemoryPool::get_instance().dealloc(obj, obj->get_size());
  }

  void sweep(void) {
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

      // TODO:

      sweep();
      ParallelChannel::get_instance().notify_sweeped(id_, objects_.size());
    }
  }
public:
  explicit Sweeper(int id)
    : id_(id)
    , mutex_()
    , cond_(mutex_)
    , thread_([this]{ sweeper_closure(); }) {
    thread_.start();
  }

  ~Sweeper(void) { stop(); thread_.join(); }
  void stop(void) { running_ = false; cond_.notify_one(); }
  void sweeping(void) { sweeping_ = true; cond_.notify_one(); }
  void put_in(BaseObject* obj) { objects_.push_back(obj); }
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
}

void ParallelChannel::clearup_sweepers(void) {
}

void* ParallelChannel::alloc(std::size_t n) {
  return Chaos::MemoryPool::get_instance().alloc(n);
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
}

void ParallelChannel::init_sweeper_marked_objects(
    int sweeper_id, std::vector<BaseObject*>& objects) {
}

void ParallelChannel::acquire_work(
    int sweeper_id, std::vector<BaseObject*>& objects) {
}

bool ParallelChannel::generate_work(int sweeper_id, BaseObject* obj) {
  return false;
}

void ParallelChannel::notify_sweeped(
    int sweeper_id, std::size_t remain_count) {
}

ParallelChannel& ParallelChannel::get_instance(void) {
  static ParallelChannel ins;
  return ins;
}

void ParallelChannel::collect(void) {
}

BaseObject* ParallelChannel::put_in(int value) {
  if (object_counter_ >= kMaxObjects)
    collect();

  auto* obj = new (alloc(sizeof(Int))) Int();
  obj->set_value(value);

  ++object_counter_;
  put_into_sweeper(obj);

  return obj;
}

BaseObject* ParallelChannel::put_in(BaseObject* first, BaseObject* second) {
  if (object_counter_ >= kMaxObjects)
    collect();

  auto* obj = new (alloc(sizeof(Pair))) Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  ++object_counter_;
  put_into_sweeper(obj);

  return obj;
}

BaseObject* ParallelChannel::fetch_out(void) {
  return nullptr;
}

}
