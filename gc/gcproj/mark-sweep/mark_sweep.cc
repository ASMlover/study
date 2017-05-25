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
#include <stdexcept>
#include "../common/object.h"
#include "mark_sweep.h"

namespace gc {

constexpr std::size_t kHeapSize = 512 << 10;

MarkSweep::MarkSweep(void) {
  heaptr_ = new uchar_t[kHeapSize];
  if (heaptr_ == nullptr)
    throw std::logic_error("allocate heap failed");
  allocptr_ = heaptr_;
}

MarkSweep::~MarkSweep(void) {
  delete [] heaptr_;
  heaptr_ = allocptr_ = nullptr;
}

uchar_t* MarkSweep::alloc(std::size_t& n) {
  if (allocptr_ + n <= heaptr_ + kHeapSize) {
    auto leftsize = static_cast<std::size_t>(
        (heaptr_ + kHeapSize) - (allocptr_ + n));
    if (leftsize > 0 && leftsize < kMinObjSize)
      n += leftsize;

    uchar_t* p = allocptr_;
    allocptr_ += n;
    return p;
  }

  uchar_t* p = heaptr_;
  while (p < allocptr_) {
    auto* scan = reinterpret_cast<MemoryHeader*>(p);
    if (scan->type() == MemoryHeader::INVALID && scan->size() >= n) {
      if (scan->size() > n && scan->size() - n < kMinObjSize)
        n = scan->size();
      return p;
    }
    else {
      p += scan->size();
    }
  }

  return nullptr;
}

Object* MarkSweep::new_object(
    std::size_t n, const std::function<Object* (uchar_t*)>& fn) {
  n = roundup(n);
  uchar_t* p = alloc(n);
  if (p == nullptr) {
    collect();
    p = alloc(n);
    if (p == nullptr)
      throw std::length_error("out of memory");
  }

  Object* obj = fn(p);
  obj->set_size(n);
  roots_.push_back(obj);
  ++objcnt_;

  return obj;
}

void MarkSweep::mark(void) {
  while (!worklist_.empty()) {
    auto* obj = worklist_.top();
    worklist_.pop();

    if (obj->type() == MemoryHeader::PAIR) {
      auto* first = static_cast<Pair*>(obj)->first();
      if (first != nullptr && !first->marked()) {
        first->set_mark();
        worklist_.push(first);
      }

      auto* second = static_cast<Pair*>(obj)->second();
      if (second != nullptr && !second->marked()) {
        second->set_mark();
        worklist_.push(second);
      }
    }
  }
}

void MarkSweep::mark_from_roots(void) {
  for (auto obj : roots_) {
    if (obj != nullptr && !obj->marked()) {
      obj->set_mark();
      worklist_.push(obj);
      mark();
    }
  }
}

void MarkSweep::sweep(void) {
  auto* p = heaptr_;
  MemoryHeader* freelist{};
  while (p < allocptr_) {
    auto* scan = reinterpret_cast<MemoryHeader*>(p);
    if (scan->type() == MemoryHeader::INVALID) {
      freelist = scan;
      p += scan->size();
      continue;
    }

    if (scan->marked()) {
      scan->unset_mark();
    }
    else {
      --objcnt_;
      scan->set_type(MemoryHeader::INVALID);
      if (freelist != nullptr) {
        if ((uchar_t*)freelist + freelist->size() == p) {
          freelist->inc_size(scan->size());
          if ((uchar_t*)freelist + freelist->size() == allocptr_) {
            allocptr_ = (uchar_t*)freelist;
            break;
          }
        }
        else {
          freelist = scan;
        }
      }
      else {
        freelist = scan;
      }

      if (p + scan->size() == allocptr_) {
        allocptr_ = p;
        break;
      }
    }
    p += scan->size();
  }
}

MarkSweep& MarkSweep::get_instance(void) {
  static MarkSweep ins;
  return ins;
}

Object* MarkSweep::new_int(int value) {
  return new_object(sizeof(Int), [value](uchar_t* p) -> Object* {
        Int* obj = new (p) Int;
        obj->set_value(value);
        return obj;
      });
}

Object* MarkSweep::new_pair(Object* first, Object* second) {
  return new_object(sizeof(Pair), [first, second](uchar_t* p) -> Object* {
        Pair* obj = new (p) Pair;
        if (first != nullptr)
          obj->set_first(first);
        if (second != nullptr)
          obj->set_second(second);
        return obj;
      });
}

Object* MarkSweep::pop_object(void) {
  Object* r = roots_.back();
  roots_.pop_back();
  return r;
}

void MarkSweep::collect(void) {
  auto old_objcnt = objcnt_;

  mark_from_roots();
  sweep();

  std::cout
    << "COLLECT OBJECT COUNT: " << old_objcnt - objcnt_
    << ", CURRENT OBJECT COUNT: " << objcnt_
    << std::endl;
}

}
