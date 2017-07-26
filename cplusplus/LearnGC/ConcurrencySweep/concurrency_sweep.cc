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
#include <Chaos/Concurrent/CurrentThread.h>
#include "object.h"
#include "concurrency_sweep.h"

namespace gc {

ConcurrencySweep::ConcurrencySweep(void)
  : thread_([this]{ sweeper_closure(); }) {
  thread_.start();
  new_objects_ = &objects_buffer_[0];
  sweep_objects_ = &objects_buffer_[1];
}

ConcurrencySweep::~ConcurrencySweep(void) {
  running_ = false;
  thread_.join();
}

void ConcurrencySweep::sweeper_closure(void) {
  while (running_) {
    collect();

    Chaos::CurrentThread::sleep_usec(1);
  }
}

void ConcurrencySweep::mark(void) {
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

void ConcurrencySweep::mark_from_roots(void) {
  for (auto* obj : marked_objects_) {
    obj->set_marked();
    worklist_.push_back(obj);
    mark();
  }
  marked_objects_.clear();
}

std::size_t ConcurrencySweep::sweep(void) {
  std::size_t counter = 0;
  for (auto it = sweep_objects_->begin(); it != sweep_objects_->end();) {
    if (!(*it)->is_marked()) {
      delete *it;
      sweep_objects_->erase(it++);
      ++counter;
    }
    else {
      (*it)->unset_marked();
      ++it;
    }
  }

  return counter;
}

void ConcurrencySweep::collect(void) {
  {
    Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
    std::swap(new_objects_, sweep_objects_);
    std::copy(roots_.begin(),
        roots_.end(), std::back_inserter(marked_objects_));
  }
  mark_from_roots();
  auto collected_count = sweep();

  auto alive_count = new_objects_->size() + sweep_objects_->size();
  std::cout
    << "[" << collected_count << "] objects collected, "
    << "[" << alive_count << "] objects aliving." << std::endl;
}


ConcurrencySweep& ConcurrencySweep::get_instance(void) {
  static ConcurrencySweep ins;
  return ins;
}

BaseObject* ConcurrencySweep::put_in(int value) {
  auto* obj = new Int();
  obj->set_value(value);

  roots_.push_back(obj);
  {
    Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
    new_objects_->push_back(obj);
  }

  return obj;
}

BaseObject* ConcurrencySweep::put_in(void) {
  BaseObject* first{};
  BaseObject* second{};
  {
    Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
    second = fetch_out();
    first = fetch_out();
  }

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  roots_.push_back(obj);
  {
    Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
    new_objects_->push_back(obj);
  }

  return obj;
}

BaseObject* ConcurrencySweep::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
