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
#include "mark_sweep.h"

namespace gc {

MarkSweep::MarkSweep(void) {
  heaptr_ = new byte_t[kHeapSize];
  CHAOS_CHECK(heaptr_, "allocating heap failed...");
  allocptr_ = heaptr_;
}

MarkSweep::~MarkSweep(void) {
  delete [] heaptr_;
  heaptr_ = allocptr_ = nullptr;
}

void* MarkSweep::alloc(std::size_t n) {
  auto index = as_index(n);
  if (freelist_[index] == nullptr) {
    auto chunk_size = as_bytes(index);
    auto alloc_size = chunk_size * kChunkCount;
    auto left_size = static_cast<std::size_t>(heaptr_ + kHeapSize - allocptr_);
    if (left_size < chunk_size)
      return nullptr;
    if (left_size < alloc_size)
      alloc_size = left_size / chunk_size * chunk_size;

    freelist_[index] = as_memory(allocptr_);
    chunklist_.push_back({index, as_memory(allocptr_)});
    allocptr_ += alloc_size;

    auto* block = new (freelist_[index]) MemoryHeader;
    for (std::size_t i = 0; i < alloc_size - chunk_size; i += chunk_size) {
      auto* next = new ((byte_t*)block + chunk_size) MemoryHeader;
      block = block->_next = next;
    }
    block->_next = nullptr;
  }

  auto* r = freelist_[index];
  freelist_[index] = r->_next;
  return r;
}

BaseObject* MarkSweep::new_object(
    std::size_t n, const std::function<BaseObject* (void*)>& fn) {
  void* p = alloc(n);
  if (p == nullptr) {
    collect();
    p = alloc(n);
    CHAOS_CHECK(p, "out of memory");
  }

  auto* obj = fn(p);
  ++obj_count_;
  roots_.push_back(obj);

  return obj;
}

void MarkSweep::mark(void) {
  while (!worklist_.empty()) {
    auto* obj = worklist_.top();
    worklist_.pop();

    if (obj->is_pair()) {
      auto* first = as_pair(obj)->first();
      if (first != nullptr && !first->is_marked()) {
        first->set_mark();
        worklist_.push(first);
      }

      auto* second = as_pair(obj)->second();
      if (second != nullptr && !second->is_marked()) {
        second->set_mark();
        worklist_.push(second);
      }
    }
  }
}

void MarkSweep::mark_from_roots(void) {
  for (auto* obj : roots_) {
    obj->set_mark();
    worklist_.push(obj);
    mark();
  }
}

void MarkSweep::sweep(void) {
  for (auto& ch : chunklist_) {
    byte_t* p = reinterpret_cast<byte_t*>(ch.chunk);
    auto block_size = as_bytes(ch.index);
    for (std::size_t i = 0;
        i < kChunkCount && p < allocptr_; ++i) {
      auto* block = as_memory(p);
      if (!block->is_invalid()) {
        if (block->is_marked()) {
          block->unset_mark();
        }
        else {
          --obj_count_;
          block->set_type(MemoryHeader::INVALID);
          block->_next = freelist_[ch.index];
          freelist_[ch.index] = block;
        }
      }
      p += block_size;
    }
  }
}

MarkSweep& MarkSweep::get_instance(void) {
  static MarkSweep ins;
  return ins;
}

void MarkSweep::collect(void) {
  auto old_count = obj_count_;

  mark_from_roots();
  sweep();

  std::cout
    << "[" << old_count - obj_count_ << "] objects collected, "
    << "[" << obj_count_ << "] remaining." << std::endl;
}

BaseObject* MarkSweep::create_int(int value) {
  return new_object(sizeof(Int), [value](void* p) -> BaseObject* {
        auto* obj = new (p) Int;
        obj->set_value(value);
        return obj;
      });
}

BaseObject* MarkSweep::create_pair(BaseObject* first, BaseObject* second) {
  return new_object(sizeof(Pair), [first, second](void* p) -> BaseObject* {
        auto* obj = new (p) Pair;
        if (first != nullptr)
          obj->set_first(first);
        if (second != nullptr)
          obj->set_second(second);
        return obj;
      });
}

BaseObject* MarkSweep::release_object(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
