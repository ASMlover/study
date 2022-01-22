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
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <Object/Object.hh>
#include <Object/StringObject.hh>

namespace Tadpole::GC {

class BaseGC : private Common::UnCopyable {
  using _Iterator     = std::pair<str_t, Object::IObjectIterator*>;
  using _IteratorList = std::vector<_Iterator>;
protected:
  _IteratorList roots_;
  std::unordered_map<u32_t, Object::StringObject*> interned_strings_;
public:
  virtual ~BaseGC() { interned_strings_.clear(); }

  virtual void collect() {}
  virtual void append_object(Object::BaseObject* o) {}
  virtual void mark_object(Object::BaseObject* o) {}

  virtual str_t get_name() const { return "BaseGC"; }
  virtual sz_t get_count() const { return 0; }
  virtual sz_t get_threshold() const { return 0; }
  virtual void set_threshold(sz_t threshold) {}
  virtual bool is_enabled() const { return true; }
  virtual void enable() {}
  virtual void disable() {}

  inline void append_roots(const str_t& name, Object::IObjectIterator* root_iterator) noexcept {
    roots_.push_back({name, root_iterator});
  }

  inline void set_interned(u32_t h, Object::StringObject* s) noexcept {
    interned_strings_[h] = s;
  }

  inline Object::StringObject* get_interned(u32_t h) const noexcept {
    if (auto it = interned_strings_.find(h); it != interned_strings_.end())
      return it->second;
    return nullptr;
  }
};

}
