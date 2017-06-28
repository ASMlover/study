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
#include "semispace_copy.h"

namespace gc {

SemispaceCopy::SemispaceCopy(void) {
  heaptr_ = new byte_t[kSemispaceSize * 2];
  CHAOS_CHECK(heaptr_ != nullptr, "create semispace failed");

  fromspace_ = heaptr_ + kSemispaceSize;
  tospace_ = heaptr_;
  allocptr_ = tospace_;
}

SemispaceCopy::~SemispaceCopy(void) {
  delete [] heaptr_;
}

void* SemispaceCopy::alloc(std::size_t n) {
  if (allocptr_ + n > tospace_ + kSemispaceSize)
    collect();
  CHAOS_CHECK(allocptr_ + n <= tospace_ + kSemispaceSize, "allocating failed");

  auto* p = allocptr_;
  allocptr_ += n;
  return p;
}

BaseObject* SemispaceCopy::worklist_fetch(void) {
  auto* ref = as_object(scanptr_);
  scanptr_ += ref->get_size();
  return ref;
}

BaseObject* SemispaceCopy::forward(BaseObject* from_ref) {
  auto* to_ref = from_ref->forward();
  if (to_ref == nullptr) {
    to_ref = copy(from_ref);
    ++obj_count_;
  }

  return Chaos::down_cast<BaseObject*>(to_ref);
}

BaseObject* SemispaceCopy::copy(BaseObject* from_ref) {
  auto* p = allocptr_;
  allocptr_ += from_ref->get_size();

  BaseObject* to_ref{};
  if (from_ref->is_int())
    to_ref = new (p) Int(std::move(*Chaos::down_cast<Int*>(from_ref)));
  else if (from_ref->is_pair())
    to_ref = new (p) Pair(std::move(*Chaos::down_cast<Pair*>(from_ref)));
  CHAOS_CHECK(to_ref != nullptr, "copy object failed");
  from_ref->set_forward(to_ref);
  worklist_put(to_ref);

  return to_ref;
}

SemispaceCopy& SemispaceCopy::get_instance(void) {
  static SemispaceCopy ins;
  return ins;
}

void SemispaceCopy::collect(void) {
  auto old_count = obj_count_;

  // flip fromspace and tospace
  std::swap(fromspace_, tospace_);
  allocptr_ = tospace_;

  obj_count_ = 0;
  worklist_init();
  for (auto& obj : roots_)
    obj = forward(obj);

  while (!worklist_empty()) {
    auto* ref = worklist_fetch();
    if (ref->is_pair()) {
      auto* pair = Chaos::down_cast<Pair*>(ref);
      auto* first = pair->first();
      if (first != nullptr)
        pair->set_first(forward(first));

      auto* second = pair->second();
      if (second != nullptr)
        pair->set_second(forward(second));
    }
  }

  std::cout
    << "[" << old_count - obj_count_ << "] objects collected, "
    << "[" << obj_count_ << "] objects remaining." << std::endl;
}

BaseObject* SemispaceCopy::put_in(int value) {
  auto* obj = new (alloc(sizeof(Int))) Int();
  obj->set_value(value);

  roots_.push_back(obj);
  ++obj_count_;

  return obj;
}

BaseObject* SemispaceCopy::put_in(BaseObject* first, BaseObject* second) {
  auto* obj = new (alloc(sizeof(Pair))) Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  roots_.push_back(obj);
  ++obj_count_;

  return obj;
}

BaseObject* SemispaceCopy::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
