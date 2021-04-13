// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___           ____    ____
// /\__  _\           /\ \                /\_ \         /\  _`\ /\  _`\
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __\ \ \L\_\ \ \/\_\
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\ \ \L_L\ \ \/_/_
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/\ \ \/, \ \ \L\ \
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\\ \____/\ \____/
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/ \/___/  \/___/
//                             \ \_\
//                              \/_/
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
#include <exception>
#include <iostream>
#include "semispace_copy.hh"

namespace tadpole::gc {

SemispaceCopy::SemispaceCopy() noexcept {
  heapptr_ = new byte_t[kSmispaceSize * 2];

  fromspace_ = heapptr_ + kSmispaceSize;
  tospace_ = heapptr_;
  freeptr_ = tospace_;
}

SemispaceCopy::~SemispaceCopy() noexcept {
  delete [] heapptr_;
}

void SemispaceCopy::collect() {
  sz_t old_objcount = objcount_;

  flip();
  worklist_init();

  objcount_ = 0;
  for (auto* ref : roots_)
    process(&ref);
  while (!worklist_empty()) {
    auto* ref = worklist_remove();
    scan(ref);
  }

  std::cout
    << "[SemispaceCopy.collect] Collected " << old_objcount - objcount_ << ", "
    << objcount_ << " remaining ..."
    << std::endl;
}

void SemispaceCopy::push_object(BaseObject* o) noexcept {
  roots_.push_back(o);
}

BaseObject* SemispaceCopy::pop_object() noexcept {
  if (!roots_.empty()) {
    BaseObject* o = roots_.back();
    roots_.pop_back();
    return o;
  }
  return nullptr;
}

BaseObject* SemispaceCopy::peek_object(int distance) const noexcept {
  int i = as_type<int>(roots_.size()) - distance - 1;
  return i < 0 ? nullptr : roots_[i];
}

void SemispaceCopy::alloc_fail() {
  std::cerr << "[SemispaceCopy] FAIL: out of memory ..." << std::endl;
  throw std::logic_error("[SemispaceCopy] FAIL: out of memory ...");
}

void* SemispaceCopy::alloc(sz_t n) {
  if (freeptr_ + n > tospace_ + kSmispaceSize) {
    collect();

    if (freeptr_ + n > tospace_ + kSmispaceSize)
      alloc_fail();
  }

  auto* p = freeptr_;
  freeptr_ += n;
  return p;
}

void SemispaceCopy::worklist_init() {
  scanptr_ = freeptr_;
}

bool SemispaceCopy::worklist_empty() const {
  return scanptr_ == freeptr_;
}

void SemispaceCopy::worklist_append(BaseObject* o) {
}

BaseObject* SemispaceCopy::worklist_remove() {
  BaseObject* ref = reinterpret_cast<BaseObject*>(scanptr_);
  scanptr_ += ref->get_size();
  return ref;
}

void SemispaceCopy::flip() {
  std::swap(fromspace_, tospace_);
  freeptr_ = tospace_;
}

void SemispaceCopy::scan(BaseObject* ref) {
  for (ObjectRef* field : ref->pointers())
    process(field);
}

void SemispaceCopy::process(ObjectRef* field) {
  auto* from_ref = *field;
  if (from_ref != nullptr)
    *field = forward(from_ref);
}

BaseObject* SemispaceCopy::forward(BaseObject* from_ref) {
  auto* to_ref = from_ref->forwarding();
  if (to_ref == nullptr)
    to_ref = copy(from_ref);
  return to_ref;
}

BaseObject* SemispaceCopy::copy(BaseObject* from_ref) {
  auto* p = freeptr_;
  if (freeptr_ + from_ref->get_size() > tospace_ + kSmispaceSize)
    alloc_fail();

  freeptr_ += from_ref->get_size();

  BaseObject* to_ref = from_ref->move_to(p);
  from_ref->set_forwarding(to_ref);
  ++objcount_;

  worklist_append(to_ref);
  return to_ref;
}

}
