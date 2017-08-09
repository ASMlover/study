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
#include <Chaos/Memory/MemoryPool.h>
#include "object.h"
#include "nursery_memory.h"

namespace gc {

void NurseryMemory::inc_nursery(BaseObject* ref) {
  if (nursery_.find(ref) != nursery_.end())
    record_objects_.push_back(ref);
}

void NurseryMemory::dec_nursery(BaseObject* ref) {
  if (nursery_.find(ref) != nursery_.end())
    record_objects_.remove(ref);
}

void NurseryMemory::write_pair(BaseObject* p, BaseObject* obj, bool is_first) {
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

BaseObject* NurseryMemory::alloc(std::uint8_t type, std::size_t bytes) {
  auto alloc_fn = [this, type, bytes](void) -> BaseObject* {
    if (nursery_.size() >= kMaxObjects || objects_.size() >= kMaxObjects)
      return nullptr;

    void* p = Chaos::MemoryPool::get_instance().alloc(bytes);
    if (type == MemoryHeader::INT)
      return new (p) Int();
    else if (type == MemoryHeader::PAIR)
      return new (p) Pair();
    return nullptr;
  };

  auto* ref = alloc_fn();
  if (ref == nullptr) {
    collect_nursery();
    ref = alloc_fn();
    if (ref == nullptr) {
      collect();
      ref = alloc_fn();
      CHAOS_CHECK(ref != nullptr, "out of memory");
    }
  }

  nursery_.insert(ref);
  return ref;
}

void NurseryMemory::dealloc(BaseObject* obj) {
  Chaos::MemoryPool::get_instance().dealloc(obj);
}

void NurseryMemory::roots_nursery(void) {
  for (auto* obj : roots_) {
    if (nursery_.find(obj) != nursery_.end())
      record_objects_.push_back(obj);
  }
}

void NurseryMemory::scan_nursery(void) {
  while (!record_objects_.empty()) {
    auto* obj = record_objects_.back();
    record_objects_.pop_back();

    if (obj->inc_ref() == 1 && obj->is_pair()) {
      auto append_fn = [this](BaseObject* o) {
        if (o != nullptr && nursery_.find(o) != nursery_.end())
          record_objects_.push_back(o);
      };

      append_fn(Chaos::down_cast<Pair*>(obj)->first());
      append_fn(Chaos::down_cast<Pair*>(obj)->second());
    }
  }
}

void NurseryMemory::sweep_nursery(void) {
  for (auto* obj : nursery_) {
    if (obj->ref() == 0) {
      dealloc(obj);
    }
    else {
      obj->set_ref(0);
      objects_.push_back(obj);
    }
  }
  nursery_.clear();
}

void NurseryMemory::roots_tracing(std::stack<BaseObject*>& trace_objects) {
  for (auto* obj : roots_)
    trace_objects.push(obj);
}

void NurseryMemory::scan_tracing(std::stack<BaseObject*>& trace_objects) {
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

void NurseryMemory::sweep_tracing(void) {
  for (auto it = objects_.begin(); it != objects_.end();) {
    if ((*it)->ref() == 0) {
      dealloc(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_ref(0);
      ++it;
    }
  }
}

NurseryMemory& NurseryMemory::get_instance(void) {
  static NurseryMemory ins;
  return ins;
}

void NurseryMemory::collect_nursery(void) {
  auto old_count = nursery_.size();

  roots_nursery();
  scan_nursery();
  sweep_nursery();

  std::cout
    << "[" << old_count - nursery_.size() << "] nursery objects collected, "
    << "[" << nursery_.size() << "] nursery objects remaining. "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

void NurseryMemory::collect(void) {
  auto old_count = objects_.size();

  std::stack<BaseObject*> trace_objects;
  roots_tracing(trace_objects);
  scan_tracing(trace_objects);
  sweep_tracing();

  std::cout
    << "[" << nursery_.size() << "] nursery objects remaining. "
    << "[" << old_count - objects_.size() << "] objects collected, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

BaseObject* NurseryMemory::put_in(int value) {
  auto* obj = Chaos::down_cast<Int*>(alloc(MemoryHeader::INT, sizeof(Int)));
  obj->set_value(value);

  roots_.push_back(obj);

  return obj;
}

BaseObject* NurseryMemory::put_in(BaseObject* first, BaseObject* second) {
  auto* obj = Chaos::down_cast<Pair*>(alloc(MemoryHeader::PAIR, sizeof(Pair)));
  if (first != nullptr)
    write_pair(obj, first, true);
  if (second != nullptr)
    write_pair(obj, second, false);

  roots_.push_back(obj);

  return obj;
}

BaseObject* NurseryMemory::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
