// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  _____         _             _
// |_   _|_ _  __| |_ __   ___ | | ___
//   | |/ _` |/ _` | '_ \ / _ \| |/ _ \
//   | | (_| | (_| | |_) | (_) | |  __/
//   |_|\__,_|\__,_| .__/ \___/|_|\___|
//                 |_|
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

#include <functional>
#include <unordered_map>
#include <vector>
#include "../object/value.hh"
#include "../object/string_object.hh"

namespace tadpole {

using MarkCallback = std::function<void ()>;

class BaseGC : private UnCopyable {
  using _Traverser      = std::pair<str_t, ObjectTraverser*>;
  using _TraverserList  = std::vector<_Traverser>;
protected:
  _TraverserList roots_;
  std::unordered_map<u32_t, StringObject*> interned_strings_;
public:
  virtual ~BaseGC() {
    interned_strings_.clear();
  }

  virtual void collect() {}
  virtual void append_object(BaseObject* o) {}
  virtual void mark_object(BaseObject* o) {}
  virtual sz_t get_threshold() const { return 0; }
  virtual void set_threshold(sz_t threshold) {}

  inline void append_roots(const str_t& name, ObjectTraverser* root) noexcept {
    roots_.push_back({name, root});
  }

  inline void set_interned(u32_t h, StringObject* s) noexcept {
    interned_strings_[h] = s;
  }

  inline StringObject* get_interned(u32_t h) const noexcept {
    if (auto it = interned_strings_.find(h); it != interned_strings_.end())
      return it->second;
    return nullptr;
  }
};

}
