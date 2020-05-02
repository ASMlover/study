// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include <InfantGC/Object.hh>
#include <InfantGC/VM.hh>

namespace _mevo::infant {

VM::VM() noexcept {
}

VM::~VM() {
  while (!objects_.empty()) {
    auto* o = objects_.back();
    objects_.pop_back();
    reclaim_object(o);
  }
}

void VM::reclaim_object(BaseObject* o) {
  std::cout << "reclaim object => " << object_asstr(o) << std::endl;
  delete o;
}

void VM::append_object(BaseObject* o) {
  if (objects_.size() > kGCThreshold)
    collect();

  objects_.push_back(o);
}

void VM::mark_object(BaseObject* o) {
  if (o == nullptr || o->marked())
    return;

  o->set_marked(true);
  worklist_.push_back(o);
}

void VM::push(BaseObject* o) {
  roots_.push_back(o);
}

BaseObject* VM::pop() {
  if (roots_.empty())
    return nullptr;

  auto* o = roots_.back();
  roots_.pop_back();
  return o;
}

BaseObject* VM::peek(sz_t distance) const {
  if (roots_.size() > distance)
    return roots_[roots_.size() - 1 - distance];
  return nullptr;
}

void VM::collect() {
  std::cout << "*** collect starting ***" << std::endl;

  // mark roots
  for (auto* o : roots_)
    mark_object(o);

  while (!worklist_.empty()) {
    auto* o = worklist_.back();
    worklist_.pop_back();
    o->blacken(*this);
  }

  // sweep
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->marked()) {
      reclaim_object(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_marked(false);
      ++it;
    }
  }

  std::cout << "*** collect finished *** [" << objects_.size() << "] objects alived" << std::endl;
}

}
