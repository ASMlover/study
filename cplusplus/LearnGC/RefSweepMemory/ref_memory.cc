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
#include "ref_memory.h"

namespace gc {

void* RefMemory::alloc(std::size_t n) {
  return Chaos::MemoryPool::get_instance().alloc(n);
}

void RefMemory::dealloc(BaseObject* obj) {
  Chaos::MemoryPool::get_instance().dealloc(obj);
}

void RefMemory::inc(BaseObject* ref) {
  if (ref != nullptr)
    inc_objects_.push(ref);
}

void RefMemory::dec(BaseObject* ref) {
  if (ref != nullptr)
    dec_objects_.push(ref);
}

void RefMemory::write(BaseObject* target, BaseObject* obj, bool is_first) {
  // only for pair object
  auto* pair = Chaos::down_cast<Pair*>(target);
  inc(obj);
  if (is_first) {
    dec(pair->first());
    pair->set_first(obj);
  }
  else {
    dec(pair->second());
    pair->set_second(obj);
  }
}

void RefMemory::apply_increments(void) {
  while (!inc_objects_.empty()) {
    auto* obj = inc_objects_.top();
    inc_objects_.pop();
    obj->inc_ref();
  }
}

void RefMemory::scan_counting(void) {
  while (!dec_objects_.empty()) {
    auto* obj = dec_objects_.top();
    dec_objects_.pop();
    if (obj->dec_def() == 0 && obj->is_pair()) {
      auto append_fn = [this](BaseObject* ob) {
        if (ob != nullptr)
          dec_objects_.push(ob);
      };

      append_fn(Chaos::down_cast<Pair*>(obj)->first());
      append_fn(Chaos::down_cast<Pair*>(obj)->second());
    }
  }
}

void RefMemory::sweep_counting(void) {
  for (auto it = objects_.begin(); it != objects_.end();) {
    if ((*it)->ref() == 0) {
      dealloc(*it);
      objects_.erase(it++);
    }
    else {
      ++it;
    }
  }
}

RefMemory& RefMemory::get_instance(void) {
  static RefMemory ins;
  return ins;
}

void RefMemory::collect_counting(void) {
  auto old_count = objects_.size();

  apply_increments();
  scan_counting();
  sweep_counting();

  std::cout
    << "[" << old_count - objects_.size() << "] objects collected, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

BaseObject* RefMemory::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect_counting();

  auto* obj = new (alloc(sizeof(Int))) Int();
  obj->set_value(value);

  roots_.push_back(obj);
  objects_.push_back(obj);
  inc(obj);

  return obj;
}

BaseObject* RefMemory::put_in(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObjects)
    collect_counting();

  auto* obj = new (alloc(sizeof(Pair))) Pair();
  if (first != nullptr)
    write(obj, first, true);
  if (second != nullptr)
    write(obj, second, false);

  roots_.push_back(obj);
  objects_.push_back(obj);
  inc(obj);

  return obj;
}

BaseObject* RefMemory::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  dec(obj);
  return obj;
}

}
