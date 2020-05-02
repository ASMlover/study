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
#pragma once

#include <list>
#include <vector>
#include <Core/MEvolve.hh>

namespace _mevo::infant {

class BaseObject;

class VM final : private UnCopyable {
  static constexpr sz_t kGCThreshold = 1 << 8;

  std::vector<BaseObject*> roots_;
  std::list<BaseObject*> objects_;
  std::list<BaseObject*> worklist_;

  VM() noexcept;
  ~VM();

  void reclaim_object(BaseObject* o);
public:
  static VM& get_instance() {
    static VM _ins;
    return _ins;
  }

  void append_object(BaseObject* o);
  void mark_object(BaseObject* o);

  void push(BaseObject* o);
  BaseObject* pop();
  BaseObject* peek(sz_t distance = 0) const;

  void collect();
};

template <typename T, typename... Args>
inline T* create_object(VM& vm, Args&&... args) {
  auto* o = new T(std::forward<Args>(args)...);
  vm.append_object(o);
  return o;
}

}
