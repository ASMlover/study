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
    record_objects_.push_back(ref);
}

void NurserySweep::dec_nursery(BaseObject* ref) {
  if (nursery_.find(ref) != nursery_.end())
    record_objects_.remove(ref);
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

BaseObject* NurserySweep::create_object(std::uint8_t type) {
  auto alloc_fn = [this](std::uint8_t type) -> BaseObject* {
    if (nursery_.size() >= kMaxObjects || objects_.size() >= kMaxObjects)
      return nullptr;

    if (type == MemoryHeader::INT)
      return new Int();
    else if (type == MemoryHeader::PAIR)
      return new Pair();
    return nullptr;
  };

  auto* ref = alloc_fn(type);
  if (ref == nullptr) {
    collect_nursery();
    ref = alloc_fn(type);
    if (ref == nullptr) {
      collect();
      ref = alloc_fn(type);
      CHAOS_CHECK(ref != nullptr, "out of memory");
    }
  }
  nursery_.insert(ref);

  return ref;
}

void NurserySweep::roots_nursery(void) {
  for (auto* obj : roots_) {
    if (nursery_.find(obj) != nursery_.end())
      record_objects_.push_back(obj);
  }
}

void NurserySweep::scan_nursery(void) {
  while (!record_objects_.empty()) {
    auto* obj = record_objects_.back();
    record_objects_.pop_back();

    if (obj->inc_ref() == 1 && obj->is_pair()) {
      auto append_fn = [this](BaseObject* obj) {
        if (obj != nullptr && nursery_.find(obj) != nursery_.end())
          record_objects_.push_back(obj);
      };

      append_fn(Chaos::down_cast<Pair*>(obj)->first());
      append_fn(Chaos::down_cast<Pair*>(obj)->second());
    }
  }
}

void NurserySweep::sweep_nursery(void) {
  for (auto* obj : nursery_) {
    if (obj->ref() == 0) {
      delete obj;
    }
    else {
      obj->set_ref(0);
      objects_.push_back(obj);
    }
  }
  nursery_.clear();
}

void NurserySweep::roots_tracing(std::stack<BaseObject*>& trace_objects) {
  for (auto* obj : roots_)
    trace_objects.push(obj);
}

void NurserySweep::scan_tracing(std::stack<BaseObject*>& trace_objects) {
  while (!trace_objects.empty()) {
    auto* obj = trace_objects.top();
    trace_objects.pop();
    if (obj->inc_ref() == 1 && obj->is_pair()) {
      auto append_fn = [&trace_objects](BaseObject* o) {
        if (o != nullptr)
          trace_objects.push(o);
      };

      append_fn(Chaos::down_cast<Pair*>(obj)->first());
      append_fn(Chaos::down_cast<Pair*>(obj)->second());
    }
  }
}

void NurserySweep::sweep_tracing(void) {
  for (auto it = objects_.begin(); it != objects_.end();) {
    if ((*it)->ref() == 0) {
      delete *it;
      objects_.erase(it++);
    }
    else {
      (*it)->set_ref(0);
      ++it;
    }
  }
}

NurserySweep& NurserySweep::get_instance(void) {
  static NurserySweep ins;
  return ins;
}

void NurserySweep::collect_nursery(void) {
  auto old_count = nursery_.size();

  roots_nursery();
  scan_nursery();
  sweep_nursery();

  std::cout
    << "[" << old_count - nursery_.size() << "] nursery objects collected, "
    << "[" << nursery_.size() << "] nursery objects remaining. "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

void NurserySweep::collect(void) {
  auto old_count = objects_.size();

  std::stack<BaseObject*> objects;
  roots_tracing(objects);
  scan_tracing(objects);
  sweep_tracing();

  std::cout
    << "[" << nursery_.size() << "] nursery objects remaining. "
    << "[" << old_count - objects_.size() << "] objects collected, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

BaseObject* NurserySweep::put_in(int value) {
  auto* obj = Chaos::down_cast<Int*>(create_object(MemoryHeader::INT));
  obj->set_value(value);
  roots_.push_back(obj);

  return obj;
}

BaseObject* NurserySweep::put_in(BaseObject* first, BaseObject* second) {
  auto* obj = create_object(MemoryHeader::PAIR);
  if (first != nullptr)
    write_pair(obj, first, true);
  if (second != nullptr)
    write_pair(obj, second, false);
  roots_.push_back(obj);

  return obj;
}

BaseObject* NurserySweep::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
