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
#include <Chaos/Base/Types.h>
#include "object.h"
#include "sweep_gc.h"

namespace gc {

void SweepGC::mark(void) {
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

void SweepGC::mark_from_roots(void) {
  for (auto* obj : roots_) {
    obj->set_marked();
    worklist_.push_back(obj);
    mark();
  }
}

void SweepGC::sweep(void) {
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

SweepGC& SweepGC::get_instance(void) {
  static SweepGC ins;
  return ins;
}

void SweepGC::collect(void) {
  auto old_count = objects_.size();

  mark_from_roots();
  sweep();

  std::cout
    << "[" << old_count - objects_.size() << "] objects collected, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

BaseObject* SweepGC::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new Int();
  obj->set_value(value);

  roots_.push_back(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* SweepGC::put_in(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObjects)
    collect();

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(obj);
  if (second != nullptr)
    obj->set_second(obj);

  roots_.push_back(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* SweepGC::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
