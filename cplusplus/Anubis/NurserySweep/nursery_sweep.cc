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
#include "object.h"
#include "nursery_sweep.h"

namespace gc {

void NurserySweep::inc_nursery(BaseObject* ref) {
  if (nursery_.find(ref) != nursery_.end())
    reclaim_objects_.push_back(ref);
}

void NurserySweep::dec_nursery(BaseObject* ref) {
  if (nursery_.find(ref) != nursery_.end())
    reclaim_objects_.remove(ref);
}

void NurserySweep::write_pair(BaseObject* p, BaseObject* obj, bool is_first) {
  auto* pair = Chaos::down_cast<Pair*>(p);
  if (nursery_.find(p) == nursery_.end()) {
    inc_nursery(obj);
    if (is_first) {
      dec_nursery(pair->first());
      pair->set_first(obj);
    }
    else {
      dec_nursery(pair->second());
      pair->set_second(obj);
    }
  }
}

void NurserySweep::roots_nursery(void) {
  for (auto* obj : roots_) {
    if (nursery_.find(obj) != nursery_.end())
      reclaim_objects_.push_back(obj);
  }
}

void NurserySweep::scan_nursery(void) {
  while (!reclaim_objects_.empty()) {
    auto* obj = reclaim_objects_.back();
    reclaim_objects_.pop_back();

    if (obj->inc_ref() == 1 && obj->is_pair()) {
      auto append_fn = [this](BaseObject* obj) {
        if (nursery_.find(obj) != nursery_.end())
          reclaim_objects_.push_back(obj);
      };

      append_fn(Chaos::down_cast<Pair*>(obj)->first());
      append_fn(Chaos::down_cast<Pair*>(obj)->second());
    }
  }
}

void NurserySweep::sweep_nursery(void) {
  for (auto it = reclaim_objects_.begin(); it != reclaim_objects_.end();) {
    if ((*it)->ref() == 0) {
      delete *it;
      reclaim_objects_.erase(it++);
    }
    else {
      ++it;
    }
  }
}

void NurserySweep::collect_nursery(void) {
  roots_nursery();
  scan_nursery();
  sweep_nursery();
}

NurserySweep& NurserySweep::get_instance(void) {
  static NurserySweep ins;
  return ins;
}

void NurserySweep::collect(void) {
}

BaseObject* NurserySweep::put_in(int value) {
  return nullptr;
}

BaseObject* NurserySweep::put_in(BaseObject* first, BaseObject* second) {
  return nullptr;
}

BaseObject* NurserySweep::fetch_out(void) {
  return nullptr;
}

}
