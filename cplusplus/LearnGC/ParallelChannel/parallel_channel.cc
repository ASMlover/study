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
#include "parallel_channel.h"

namespace gc {

class Tracer : private Chaos::UnCopyable {
  int id_{};
  bool running_{true};
  bool tracing_{};
  mutable Chaos::Mutex mutex_;
  Chaos::Condition cond_;
  Chaos::Thread thread_;
  std::vector<BaseObject*> marked_objects_;

  void perform_work(void) {
    while (!marked_objects_.empty()) {
      auto* obj = marked_objects_.back();
      marked_objects_.pop_back();

      obj->set_marked();
      if (obj->is_pair()) {
        auto append_fn = [this](BaseObject* o) {
          if (o != nullptr && !o->is_marked()) {
            if (!ParallelChannel::get_instance().generate_work(id_, o))
              marked_objects_.push_back(o);
          }
        };

        append_fn(Chaos::down_cast<Pair*>(obj)->first());
        append_fn(Chaos::down_cast<Pair*>(obj)->second());
      }
    }
  }

  void tracer_closure(void) {
    while (running_) {
      {
        Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
        while (running_ && !tracing_)
          cond_.wait();
        if (!running_)
          break;
      }

      while (tracing_) {
        ParallelChannel::get_instance().acquire_work(id_, marked_objects_);
        perform_work();

        if (marked_objects_.empty()) {
          tracing_ = false;
          ParallelChannel::get_instance().notify_sweeping(id_);
        }
      }
    }
  }
public:
  explicit Tracer(int id)
    : id_(id)
    , mutex_()
    , cond_(mutex_)
    , thread_([this]{ tracer_closure(); }) {
    thread_.start();
  }

  ~Tracer(void) { stop(); thread_.join(); }
  void stop(void) { running_ = false; cond_.notify_one(); }
  void tracing(void) { tracing_ = true; cond_.notify_one(); }
};

ParallelChannel::ParallelChannel(void)
  : trace_mutex_()
  , trace_cond_(trace_mutex_) {
  startup_tracers();
}

ParallelChannel::~ParallelChannel(void) {
  clearup_tracers();
}

void ParallelChannel::startup_tracers(void) {
  for (auto i = 0; i < kMaxTraceers; ++i)
    tracers_.emplace_back(new Tracer(i));
}

void ParallelChannel::clearup_tracers(void) {
  for (auto& t : tracers_)
    t->stop();
}

int ParallelChannel::put_in_order(void) {
  auto r = order_;
  order_ = (order_ + 1) % kMaxTraceers;
  return r;
}

int ParallelChannel::fetch_out_order(void) {
  order_ = (order_ - 1 + kMaxTraceers) % kMaxTraceers;
  return order_;
}

void ParallelChannel::real_put_in(BaseObject* obj) {
  auto i = put_in_order();
  channels_[i][i].push_back(obj);
}

void ParallelChannel::acquire_work(
    int tracer_id, std::vector<BaseObject*>& objects) {
  for (auto i = 0; i < kMaxTraceers; ++i) {
    if (i == tracer_id)
      continue;
    if (!channels_[i][tracer_id].empty()) {
      auto* obj = channels_[i][tracer_id].back();
      channels_[i][tracer_id].pop_back();
      objects.push_back(obj);
      break;
    }
  }
}

bool ParallelChannel::generate_work(int tracer_id, BaseObject* obj) {
  for (auto i = 0; i < kMaxTraceers; ++i) {
    if (i == tracer_id)
      continue;
    if (channels_[i][tracer_id].size() < kChannelObjects) {
      channels_[i][tracer_id].push_back(obj);
      return true;
    }
  }
  return false;
}

void ParallelChannel::notify_sweeping(int /*id*/) {
  {
    Chaos::ScopedLock<Chaos::Mutex> g(trace_mutex_);
    ++tracer_counter_;
  }
  trace_cond_.notify_one();
}

void ParallelChannel::sweep(void) {
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

ParallelChannel& ParallelChannel::get_instance(void) {
  static ParallelChannel ins;
  return ins;
}

void ParallelChannel::collect(void) {
  auto old_count = objects_.size();

  tracer_counter_ = 0;
  for (auto& t : tracers_)
    t->tracing();

  {
    Chaos::ScopedLock<Chaos::Mutex> g(trace_mutex_);
    while (tracer_counter_ < kMaxTraceers)
      trace_cond_.wait();
  }

  sweep();

  std::cout
    << "[" << old_count - objects_.size() << "] objects collected, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

BaseObject* ParallelChannel::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new Int();
  obj->set_value(value);

  real_put_in(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelChannel::put_in(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  real_put_in(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* ParallelChannel::fetch_out(void) {
  auto i = fetch_out_order();
  if (!channels_[i][i].empty()) {
    auto* obj = channels_[i][i].back();
    channels_[i][i].pop_back();
    return obj;
  }
  return nullptr;
}

}
