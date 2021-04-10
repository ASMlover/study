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
#include <iostream>
#include "object.hh"
#include "mark_sweep.hh"

namespace tadpole::gc {

MarkSweep::MarkSweep() noexcept {
}

MarkSweep::~MarkSweep() noexcept {
  while (!objects_.empty()) {
    auto* o = objects_.back();
    objects_.pop_back();

    reclaim(o);
  }
}

void MarkSweep::collect() {
  sz_t old_objcount = objects_.size();

  mark_from_roots();
  sweep();

  sz_t remaining_objcount = objects_.size();
  gc_threshold_ = as_align(objects_.empty() ? kGCThreshold : remaining_objcount * kGCFactor, kAlignment);

  std::cout
    << "[MarkSweep.coolect] Collected " << old_objcount - remaining_objcount << ", "
    << remaining_objcount << " remaining ..."
    << std::endl;
}

void MarkSweep::reclaim(BaseObject* o) {
  // std::cout << "[" << o << "] reclaim object: `" << o->get_name() << "`" << std::endl;

  delete o;
}

void MarkSweep::mark_from_roots() {
  worklist_.clear();

  for (auto* ref : roots_) {
    if (ref != nullptr && !ref->is_marked()) {
      ref->set_marked(true);
      worklist_.push_back(ref);
      mark();
    }
  }
}

void MarkSweep::mark() {
  while (!worklist_.empty()) {
    auto* ref = worklist_.back();
    worklist_.pop_back();

    for (auto* child : ref->pointers()) {
      if (child != nullptr && !child->is_marked()) {
        child->set_marked(true);
        worklist_.push_back(child);
      }
    }
  }
}

void MarkSweep::sweep() {
  auto scan = objects_.begin();
  while (scan != objects_.end()) {
    if ((*scan)->is_marked()) {
      (*scan)->set_marked(false);
      ++scan;
    }
    else {
      reclaim(*scan);
      objects_.erase(scan++);
    }
  }
}

}
