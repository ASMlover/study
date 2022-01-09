// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
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
#include <iostream>
#include <Common/Colorful.hh>
#include <GC/MarkSweep.hh>

namespace Tadpole::GC {

MarkSweep::MarkSweep() noexcept {
}

MarkSweep::~MarkSweep() {
  while (!objects_.empty()) {
    auto* o = objects_.back();
    objects_.pop_back();

    reclaim_object(o);
  }
}

void MarkSweep::collect() {
  mark_from_roots();
  sweep();

  gc_threshold_ = std::max(std::min(gc_threshold_, kGCThresholdMin), objects_.size() * kGCFactor);
  gc_threshold_ = Common::as_align(gc_threshold_, kGCAlign);
}

void MarkSweep::append_object(Object::BaseObject* o) {
  if (objects_.size() >= gc_threshold_)
    collect();
  objects_.push_back(o);
}

void MarkSweep::mark_object(Object::BaseObject* o) {}
sz_t MarkSweep::get_count() const { return 0; }
sz_t MarkSweep::get_threshold() const { return 0; }
void MarkSweep::set_threshold(sz_t threshold) {}

void MarkSweep::mark() {}
void MarkSweep::mark_from_roots() {}
void MarkSweep::sweep() {}
void MarkSweep::reclaim_object(Object::BaseObject* o) {}

}
