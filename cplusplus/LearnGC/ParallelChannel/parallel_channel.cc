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

  void perform_work(void) {
    while (!marked_objects_.empty()) {
      auto* obj = marked_objects_.back();
      marked_objects_.pop_back();
      if (obj == nullptr)
        continue;

      if (!obj->is_marked())
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

      marked_objects_.clear();
      ParallelChannel::get_instance().init_marked_objects(
          id_, marked_objects_);

      while (sweeping_ || !marked_objects_.empty()) {
        ParallelChannel::get_instance().acquire_work(id_, marked_objects_);
        perform_work();

        if (marked_objects_.empty()) {
          sweeping_ = false;
          break;
        }
      }

      sweep();
      ParallelChannel::get_instance().notify_sweeping(id_, objects_.size());
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
  for (auto i = 0; i < kMaxSweepers; ++i)
    sweepers_.emplace_back(new Sweeper(i));
}

void ParallelChannel::clearup_sweepers(void) {
  for (auto& s : sweepers_)
    s->stop();
}

int ParallelChannel::put_in_order(void) {
  auto r = order_;
  order_ = (order_ + 1) % kMaxSweepers;
  return r;
}

int ParallelChannel::fetch_out_order(void) {
  order_ = (order_ - 1 + kMaxSweepers) % kMaxSweepers;
  return order_;
}

void ParallelChannel::put_into_sweeper(BaseObject* obj) {
  auto i = put_in_order();
  channels_[i][i].push_back(obj);
  sweepers_[i]->put_in(obj);
}

void ParallelChannel::init_marked_objects(
    int sweeper_id, std::vector<BaseObject*>& objects) {
  auto& ch = channels_[sweeper_id][sweeper_id];
  if (!ch.empty())
    std::copy(ch.begin(), ch.end(), std::back_inserter(objects));
}

void ParallelChannel::acquire_work(
    int sweeper_id, std::vector<BaseObject*>& objects) {
  for (auto i = 0; i < kMaxSweepers; ++i) {
    if (i == sweeper_id)
      continue;
    if (!channels_[i][sweeper_id].empty()) {
      auto* obj = channels_[i][sweeper_id].back();
      channels_[i][sweeper_id].pop_back();
      objects.push_back(obj);
      break;
    }
  }
}

bool ParallelChannel::generate_work(int sweeper_id, BaseObject* obj) {
  for (auto i = 0; i < kMaxSweepers; ++i) {
    if (i == sweeper_id)
      continue;
    if (channels_[sweeper_id][i].size() < kChannelObjects) {
      channels_[sweeper_id][i].push_back(obj);
      return true;
    }
  }
  return false;
}

void ParallelChannel::notify_sweeping(int /*id*/, std::size_t remain_count) {
  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweeper_mutex_);
    ++sweeper_counter_;
    object_counter_ += remain_count;
  }
  sweeper_cond_.notify_one();
}

ParallelChannel& ParallelChannel::get_instance(void) {
  static ParallelChannel ins;
  return ins;
}

void ParallelChannel::collect(void) {
  auto old_count = object_counter_;

  sweeper_counter_ = 0;
  object_counter_ = 0;
  for (auto& s : sweepers_)
    s->sweeping();

  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweeper_mutex_);
    while (sweeper_counter_ < kMaxSweepers)
      sweeper_cond_.wait();
  }

  std::cout
    << "[" << old_count - object_counter_ << "] objects collected, "
    << "[" << object_counter_ << "] objects remaining." << std::endl;
}

BaseObject* ParallelChannel::put_in(int value) {
  if (object_counter_ >= kMaxObjects)
    collect();

  auto* obj = new Int();
  obj->set_value(value);

  ++object_counter_;
  put_into_sweeper(obj);

  return obj;
}

BaseObject* ParallelChannel::put_in(BaseObject* first, BaseObject* second) {
  if (object_counter_ >= kMaxObjects)
    collect();

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  ++object_counter_;
  put_into_sweeper(obj);

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
