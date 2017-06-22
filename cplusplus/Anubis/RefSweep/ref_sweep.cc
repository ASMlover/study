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
#include "object.h"
#include "ref_sweep.h"

namespace gc {

void RefSweep::inc_ref(BaseObject* obj) {
  if (obj != nullptr)
    inc_objects_.push(obj);
}

void RefSweep::dec_ref(BaseObject* obj) {
  if (obj != nullptr)
    dec_objects_.push(obj);
}

void RefSweep::write(BaseObject* pair, BaseObject* obj, bool is_first) {
  auto* dest = as_pair(pair);
  inc_ref(obj);
  if (is_first) {
    dec_ref(dest->first());
    dest->set_first(obj);
  }
  else {
    dec_ref(dest->second());
    dest->set_second(obj);
  }
}

void RefSweep::apply_increments(void) {
  while (!inc_objects_.empty()) {
    auto* obj = inc_objects_.top();
    inc_objects_.pop();
    obj->inc_ref();
  }
}

void RefSweep::scan_counting(void) {
  while (!dec_objects_.empty()) {
    auto* obj = dec_objects_.top();
    dec_objects_.pop();
    obj->dec_ref();
    if (obj->ref() == 0 && obj->is_pair()) {
      auto* first = as_pair(obj)->first();
      if (first != nullptr)
        dec_objects_.push(first);

      auto* second = as_pair(obj)->second();
      if (second != nullptr)
        dec_objects_.push(second);
    }
  }
}

void RefSweep::sweep_counting(void) {
  for (auto it = objects_.begin(); it != objects_.end();) {
    if ((*it)->ref() == 0) {
      delete *it;
      objects_.erase(it++);
    }
    else {
      ++it;
    }
  }
}

RefSweep& RefSweep::get_instance(void) {
  static RefSweep ins;
  return ins;
}

void RefSweep::collect_counting(void) {
  auto old_count = objects_.size();

  apply_increments();
  scan_counting();
  sweep_counting();

  std::cout
    << "[" << old_count - objects_.size() << "] objects colleced, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

BaseObject* RefSweep::create_int(int value) {
  if (objects_.size() >= kMaxObject)
    collect_counting();

  auto* obj = new Int();
  obj->set_value(value);

  objects_.push_back(obj);
  roots_.push_back(obj);
  inc_ref(obj);

  return obj;
}

BaseObject* RefSweep::create_pair(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObject)
    collect_counting();

  auto* obj = new Pair();
  if (first != nullptr)
    write(obj, first, true);
  if (second != nullptr)
    write(obj, second, false);

  objects_.push_back(obj);
  roots_.push_back(obj);
  inc_ref(obj);

  return obj;
}

BaseObject* RefSweep::release_object(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  dec_ref(obj);
  return obj;
}

}
