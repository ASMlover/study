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
#include "concurrency_copy.h"

namespace gc {

ConcurrencyCopy::ConcurrencyCopy(void)
  : thread_([this]{ concurrency_closure(); }) {
  heaptr_ = new byte_t[kSemispaceSize * 2];
  CHAOS_CHECK(heaptr_ != nullptr, "create semispace failed");

  fromspace_ = heaptr_ + kSemispaceSize;
  tospace_ = heaptr_;
  allocptr_ = tospace_;

  thread_.start();
}

ConcurrencyCopy::~ConcurrencyCopy(void) {
  running_ = false;
  thread_.join();
}

void* ConcurrencyCopy::alloc(std::size_t n) {
  auto* p = allocptr_;
  allocptr_ += n;
  CHAOS_CHECK(allocptr_ <= tospace_ + kSemispaceSize, "out of memory");

  return p;
}

void ConcurrencyCopy::concurrency_closure(void) {
  // TODO:
  while (running_) {
  }
}

BaseObject* ConcurrencyCopy::forward(BaseObject* from_ref) {
  auto* to_ref = from_ref->forward();
  if (to_ref == nullptr)
    to_ref = copy(from_ref);
  return Chaos::down_cast<BaseObject*>(to_ref);
}

BaseObject* ConcurrencyCopy::copy(BaseObject* from_ref) {
  auto* p = allocptr_;
  allocptr_ += from_ref->get_size();
  CHAOS_CHECK(allocptr_ <= tospace_ + kSemispaceSize, "out of memory");

  BaseObject* to_ref{};
  if (from_ref->is_int())
    to_ref = new (p) Int(std::move(*Chaos::down_cast<Int*>(from_ref)));
  else if (from_ref->is_pair())
    to_ref = new (p) Pair(std::move(*Chaos::down_cast<Pair*>(from_ref)));
  CHAOS_CHECK(to_ref != nullptr, "copy object failed");
  from_ref->set_forward(to_ref);

  worklist_.push_back(to_ref);
  return to_ref;
}

void ConcurrencyCopy::collect(void) {
  {
    Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
    // semispace flip
    std::swap(fromspace_, tospace_);
    allocptr_ = tospace_;

    for (auto& obj : roots_)
      obj = forward(obj);
  }

  while (true) {
    Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
    if (worklist_.empty())
      break;

    auto* obj = worklist_.back();
    worklist_.pop_back();
    if (obj->is_pair()) {
      auto* pair = Chaos::down_cast<Pair*>(obj);
      auto* first = pair->first();
      if (first != nullptr)
        pair->set_first(forward(first));

      auto* second = pair->second();
      if (second != nullptr)
        pair->set_second(forward(second));
    }
  }
}

ConcurrencyCopy& ConcurrencyCopy::get_instance(void) {
  static ConcurrencyCopy ins;
  return ins;
}

BaseObject* ConcurrencyCopy::put_in(int value) {
  auto* obj = new (alloc(sizeof(Int))) Int();
  obj->set_value(value);

  roots_.push_back(obj);
  return obj;
}

BaseObject* ConcurrencyCopy::put_in(void) {
  auto* obj = new (alloc(sizeof(Pair))) Pair();
  // TODO:
  return nullptr;
}

BaseObject* ConcurrencyCopy::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
