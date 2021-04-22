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
#include <memory>
#include <vector>
#include <tadpole/common/common.hh>

namespace tadpole::gc {

class BaseObject;
using ObjectRef = BaseObject*;

class BaseGC : private UnCopyable {
protected:
  std::vector<BaseObject*> roots_;
public:
  inline void push_object(BaseObject* o) noexcept {
    roots_.push_back(o);
  }

  inline BaseObject* pop_object() noexcept {
    if (!roots_.empty()) {
      BaseObject* o = roots_.back();
      roots_.pop_back();
      return o;
    }
    return nullptr;
  }

  inline BaseObject* peek_object(int distance = 0) const noexcept {
    int i = as_type<int>(roots_.size()) - distance - 1;
    return i < 0 ? nullptr : roots_[i];
  }

  virtual void* allocate(sz_t size) noexcept { return std::malloc(size); }
  virtual void deallocate(void* p) noexcept { std::free(p); }
  virtual void set_object(ObjectRef* target, BaseObject* source) noexcept { *target = source; }

  virtual void collect() {}
};

class GlobalGC final : public Singleton<GlobalGC> {
  std::shared_ptr<BaseGC> gc_{};
public:
  inline bool can_usable() const noexcept { return gc_ != nullptr; }
  inline BaseGC& get_gc() noexcept  { return *gc_; }
  inline const BaseGC& get_gc() const noexcept { return *gc_; }

  void switch_gc(const str_t& gc_name) noexcept;

  template <typename Object, typename... Args> inline Object* create_object(Args&&... args) noexcept {
    if (!can_usable())
      return nullptr;

    auto& gc = get_gc();
    void* p = gc.allocate(sizeof(Object));
    if (p == nullptr) {
      gc.collect();

      p = gc.allocate(sizeof(Object));
      if (p == nullptr)
        throw std::logic_error("GlobalGC: Out of Memory ...");
    }

    Object* o = new (p) Object(std::forward<Args>(args)...);
    return o;
  }

  template <typename Object> void reclaim(Object* o) noexcept {
    if (!can_usable())
      return;

    o->~Object();
    get_gc().deallocate(o);
  }
};

}
