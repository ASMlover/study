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
#include <Chaos/Memory/MemoryPool.h>
#include "object.h"
#include "delay_memory.h"

namespace gc {

void* DelayMemory::alloc(std::size_t n) {
  return Chaos::MemoryPool::get_instance().alloc(n);
}

void DelayMemory::dealloc(BaseObject* obj) {
  if (obj != nullptr) {
    if (obj->is_int())
      Chaos::MemoryPool::get_instance().dealloc(obj, sizeof(Int));
    else if (obj->is_pair())
      Chaos::MemoryPool::get_instance().dealloc(obj, sizeof(Pair));
  }
}

void DelayMemory::inc(BaseObject* ref) {
  if (ref != nullptr)
    inc_objects_.push(ref);
}

void DelayMemory::dec(BaseObject* ref) {
  if (ref != nullptr)
    dec_objects_.push(ref);
}

void DelayMemory::write(BaseObject* target, BaseObject* obj, bool is_first) {
  auto* pair = as_pair(target);
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

void DelayMemory::roots_tracing(std::stack<BaseObject*>& trace_objects) {
}

void DelayMemory::apply_increments(void) {
}

void DelayMemory::apply_decrements(void) {
}

void DelayMemory::scan_counting(void) {
}

void DelayMemory::sweep_counting(void) {
}

DelayMemory& DelayMemory::get_instance(void) {
  static DelayMemory ins;
  return ins;
}

void DelayMemory::collect_counting(void) {
}

BaseObject* DelayMemory::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect_counting();

  auto* obj = new (alloc(sizeof(Int))) Int();
  obj->set_value(value);

  roots_.push_back(obj);
  objects_.push_back(obj);
  inc(obj);

  return obj;
}

BaseObject* DelayMemory::put_in(BaseObject* first, BaseObject* second) {
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

BaseObject* DelayMemory::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  dec(obj);
  return obj;
}

}
