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

void MarkSweep::mark_object(Object::BaseObject* o) {
  if (o == nullptr || o->is_marked())
    return;

#if defined(_TADPOLE_DEBUG_GC)
  std::cout
    << Common::Colorful::fg::lightcyan
    << "[" << o << "] mark object: `" << o->stringify() << "`"
    << Common::Colorful::reset
    << std::endl;
#endif

  o->set_marked(true);
  worklist_.push_back(o);
}

sz_t MarkSweep::get_count() const {
  return objects_.size();
}

sz_t MarkSweep::get_threshold() const {
  return gc_threshold_;
}

void MarkSweep::set_threshold(sz_t threshold) {
  gc_threshold_ = Common::as_align(threshold, kGCAlign);
}

bool MarkSweep::is_enabled() const {
  return is_enabled_;
}

void MarkSweep::enable() {
  is_enabled_ = true;
}

void MarkSweep::disable() {
  is_enabled_ = false;
}

void MarkSweep::mark() {
  while (!worklist_.empty()) {
    Object::BaseObject* o = worklist_.back();
    worklist_.pop_back();

    o->iter_children([this](Object::BaseObject* child) { mark_object(child); });
  }
}

void MarkSweep::mark_from_roots() {
  for (auto& r : roots_) {
    r.second->iter_objects([this](Object::BaseObject* o) {
          mark_object(o);
          mark();
        });
  }

  for (auto& s : interned_strings_) {
    mark_object(s.second);
    mark();
  }
}

void MarkSweep::sweep() {
  for (auto it = interned_strings_.begin(); it != interned_strings_.end();) {
    if (!it->second->is_marked())
      interned_strings_.erase(it++);
    else
      ++it;
  }

  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->is_marked()) {
      reclaim_object(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_marked(true);
      ++it;
    }
  }
}

void MarkSweep::reclaim_object(Object::BaseObject* o) {
#if defined(_TADPOLE_DEBUG_GC)
  std::cout
    << Common::Colorful::fg::gray
    << "[" << o << "] reclaim object type: `" << o->type_asstr() << "`"
    << Common::Colorful::reset
    << std::endl;
#endif

  delete o;
}

}
