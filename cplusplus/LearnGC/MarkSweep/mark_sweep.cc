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
#include <Chaos/Except/SystemError.h>
#include "object.h"
#include "mark_sweep.h"

namespace gc {

MarkSweep::MarkSweep(void) {
  heaptr_ = new byte_t[kHeapSize];
  CHAOS_CHECK(heaptr_ != nullptr, "allocting heap failed");
  allocptr_ = heaptr_;
}

MarkSweep::~MarkSweep(void) {
  delete [] heaptr_;
}

byte_t* MarkSweep::alloc(std::size_t& n) {
  if (allocptr_ + n <= heaptr_ + kHeapSize) {
    auto leftsize =
      static_cast<std::size_t>((heaptr_ + kHeapSize) - (allocptr_ + n));
    if (leftsize > 0 && leftsize < kMinObjSize)
      n += leftsize;

    byte_t* p = allocptr_;
    allocptr_ += n;
    return p;
  }

  byte_t* p = heaptr_;
  while (p < allocptr_) {
    auto* scan = as_object(p);
    if (scan->is_invalid() && scan->size() >= n) {
      if (scan->size() - n < kMinObjSize || scan->size() == n) {
        n = scan->size();
      }
      else {
        auto* mem = new (p + n) MemoryHeader;
        mem->set_size(scan->size() - n);
      }
      return p;
    }
    else {
      p += scan->size();
    }
  }

  return nullptr;
}

BaseObject* MarkSweep::create_object(
      std::size_t n, std::function<BaseObject* (byte_t*)>&& fn) {
  n = roundup(n);
  byte_t* p = alloc(n);
  if (p == nullptr) {
    collect();
    p = alloc(n);
    CHAOS_CHECK(p != nullptr, "out of memory");
  }

  auto* obj = fn(p);
  obj->set_size(n);
  roots_.push_back(obj);
  ++obj_count_;

  return obj;
}

void MarkSweep::mark(void) {
  while (!worklist_.empty()) {
    auto* obj = worklist_.top();
    worklist_.pop();

    if (obj->is_pair()) {
      auto push_fn = [this](BaseObject* o) {
        if (o != nullptr && !o->is_marked()) {
          o->set_mark(); worklist_.push(o);
        }
      };

      push_fn(Chaos::down_cast<Pair*>(obj)->first());
      push_fn(Chaos::down_cast<Pair*>(obj)->second());
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
  auto* p = heaptr_;
  MemoryHeader* freelist{};
  while (p < allocptr_) {
    auto* scan = as_object(p);
    if (!scan->is_invalid()) {
      if (scan->is_marked()) {
        scan->unset_mark();
      }
      else {
        --obj_count_;
        scan->set_type(MemoryHeader::INVALID);
        if (freelist != nullptr) {
          if ((byte_t*)freelist + freelist->size() == p) {
            freelist->inc_size(scan->size());
            if ((byte_t*)freelist + freelist->size() == allocptr_) {
              allocptr_ = (byte_t*)freelist;
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
    }

    p += scan->size();
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

BaseObject* MarkSweep::put_in(int value) {
  return create_object(sizeof(Int), [value](byte_t* p) -> BaseObject* {
        auto* obj = new (p) Int();
        obj->set_value(value);
        return obj;
      });
}

BaseObject* MarkSweep::put_in(BaseObject* first, BaseObject* second) {
  return create_object(sizeof(Pair), [first, second](byte_t* p) -> BaseObject* {
        auto* obj = new (p) Pair();
        if (first != nullptr)
          obj->set_first(first);
        if (second != nullptr)
          obj->set_second(second);
        return obj;
      });
}

BaseObject* MarkSweep::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
