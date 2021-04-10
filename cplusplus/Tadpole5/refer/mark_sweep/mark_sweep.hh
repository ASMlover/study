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
#pragma once

#include <exception>
#include <iostream>
#include <list>
#include <vector>
#include <tadpole/common/common.hh>
#include "object.hh"

namespace tadpole::gc {

class BaseObject;

class MarkSweep final : public Singleton<MarkSweep> {
  static constexpr sz_t kAlignment    = 1 << 3;
  static constexpr sz_t kGCThreshold  = 1 << 10;
  static constexpr sz_t kGCFactor     = 2;

  sz_t gc_threshold_{kGCThreshold};
  std::vector<BaseObject*> roots_;
  std::list<BaseObject*> objects_;
  std::list<BaseObject*> worklist_;

  void reclaim(BaseObject* o);

  void mark_from_roots();
  void mark();
  void sweep();
public:
  MarkSweep() noexcept;
  ~MarkSweep() noexcept;

  void collect();

  void push_object(BaseObject* o) noexcept;
  BaseObject* pop_object() noexcept;
  BaseObject* peek_object(int distance = 0) const noexcept;

  template <typename Object, typename... Args> inline Object* create_object(Args&&... args) {
    if (objects_.size() >= gc_threshold_) {
      collect();

      if (objects_.size() >= gc_threshold_)
        throw std::logic_error("[MarkSweep] FAIL: out of memory ...");
    }

    Object* o = new Object(std::forward<Args>(args)...);
    objects_.push_back(o);
    return o;
  }
};

}
