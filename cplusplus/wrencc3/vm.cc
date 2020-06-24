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
#include "vm.hh"

namespace wrencc {

WrenVM::WrenVM() noexcept {}
WrenVM::~WrenVM() {}

void WrenVM::collect() {}

void WrenVM::append_object(BaseObject* obj) {
  if (all_objects_.size() > next_gc_) {
    collect();
    next_gc_ = totoal_allocted_ * 3 / 2;
  }
  all_objects_.push_back(obj);
}

// mark [obj] as reachable and still in use, this should only be called
// during the sweep phase of a garbage collection
void WrenVM::gray_object(BaseObject* obj) {
  if (obj == nullptr)
    return;

  // stop if the object is already darkened so we don't get stuck in a cycle
  if (obj->is_darken())
    return;

  // it's been reached
  obj->set_darken(true);

  // add it to the gray list so it can be recursively explored for more
  // marks later
  if (gray_objects_.size() >= gray_capacity_) {
    gray_capacity_ = gray_objects_.size() * 2;
    gray_objects_.reserve(gray_capacity_);
  }
  gray_objects_.push_back(obj);
}

// mark [val] as reachable and still in use, this should only be called
// during the sweep phase of a garbage collection
void WrenVM::gray_value(const Value& val) {
  if (val.is_object())
    gray_value(val.as_object());
}

}
