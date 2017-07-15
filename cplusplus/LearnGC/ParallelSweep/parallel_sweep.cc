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
#include <mutex>
#include <thread>
#include <Chaos/Types.h>
#include "object.h"
#include "parallel_sweep.h"

namespace gc {

struct Worker {
  std::mutex mutex_;
  std::unique_ptr<std::thread> thread_;
  std::vector<BaseObject*> roots_;
public:
  Worker(std::thread* thread)
    : thread_(thread) {
  }

  void put_in(BaseObject* obj) {
    roots_.push_back(obj);
  }

  BaseObject* fetch_out(void) {
    auto* obj = roots_.back();
    roots_.pop_back();
    return obj;
  }

  void transfer(std::size_t n, std::vector<BaseObject*>& objects) {
    std::copy_n(roots_.begin(), n, objects.begin());
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
  for (auto i = 0; i < nworkers_; ++i) {
    auto* worker = new Worker(new std::thread(
        std::bind(&ParallelSweep::collect_routine, this)));
    workers_.emplace_back(worker);
  }
}

void ParallelSweep::stop_workers(void) {
  stop_ = true;
  for (auto i = 0; i < nworkers_; ++i)
    workers_[i]->thread_->join();
}

void ParallelSweep::collect_routine(void) {
  std::vector<BaseObject*> stack_objects;
  while (!stop_) {
  }
}

int ParallelSweep::put_in_turn(void) {
  int index = put_index_;
  put_index_ = (put_index_ + 1) % nworkers_;
  return index;
}

int ParallelSweep::fetch_out_turn(void) {
  int index = fetch_index_;
  fetch_index_ = (fetch_index_ + 1) % nworkers_;
  return index;
}

void ParallelSweep::acquire_work(void) {
}

void ParallelSweep::perform_work(void) {
}

void ParallelSweep::generate_work(void) {
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

void ParallelSweep::collect(void) {
}

BaseObject* ParallelSweep::put_in(int value) {
  // TODO:

  auto* obj = new Int();
  obj->set_value(value);

  // TODO:

  return obj;
}

BaseObject* ParallelSweep::put_in(BaseObject* first, BaseObject* second) {
  // TODO:

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  // TODO:

  return obj;
}

BaseObject* ParallelSweep::fetch_out(void) {
  // TODO:
  return nullptr;
}

}
