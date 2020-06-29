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
#include "vm.hh"

namespace wrencc {

WrenVM::WrenVM() noexcept {}
WrenVM::~WrenVM() {}

void WrenVM::print_stacktrace() {}
void WrenVM::runtime_error() {}

void WrenVM::free_object(BaseObject* obj) {
#if defined(WRENCC_TRACE_MEMORY)
  std::cout
    << "`" << Xt::cast<void>(obj) << "` free object"
# if defined(WRENCC_TRACE_OBJECT_DETAIL)
    << " `" << obj->stringify() << "`"
# endif
    << std::endl;
#endif

  obj->finalize(*this);
  delete obj;
}

void WrenVM::push_root(BaseObject* obj) {
  ASSERT(obj != nullptr, "cannot root nullptr");
  ASSERT(temp_roots_.size() < kMaxTempRoots, "too many temporary roots");

  temp_roots_.push_back(obj);
}

void WrenVM::pop_root() {
  ASSERT(temp_roots_.size() > 0, "no temporary roots to release");

  temp_roots_.pop_back();
}

void WrenVM::collect() {}

void WrenVM::append_object(BaseObject* obj) {
  if (all_objects_.size() > next_gc_) {
    collect();
    next_gc_ = totoal_allocted_ * 3 / 2;
  }
  all_objects_.push_back(obj);
}

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

void WrenVM::gray_value(const Value& val) {
  if (val.is_object())
    gray_object(val.as_object());
}

void WrenVM::blacken_objects() {
  while (!gray_objects_.empty()) {
    // pop an item from the gray stack
    BaseObject* obj = gray_objects_.back();
    gray_objects_.pop_back();

    obj->gc_blacken(*this);
  }
}

InterpretRet WrenVM::interpret(const str_t& module, const str_t& source_bytes) {
  // TODO:
  return InterpretRet::SUCCESS;
}

}
