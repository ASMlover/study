// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef CHAOS_MEMORY_INTRUSIVEPTR_H
#define CHAOS_MEMORY_INTRUSIVEPTR_H

#include <Chaos/Memory/IntrusiveRefCounter.h>

namespace Chaos {

template <typename T>
class IntrusivePtr {
  T* px_{};

  typedef IntrusivePtr<T> SelfType;
public:
  IntrusivePtr(void) = default;

  IntrusivePtr(T* p, bool add_ref = true)
    : px_(p) {
    if (nullptr != px_ && add_ref)
      intrusive_ptr_add_ref(px_);
  }

  ~IntrusivePtr(void) {
    if (nullptr != px_)
      intrusive_ptr_del_ref(px_);
  }

  IntrusivePtr(const IntrusivePtr& r)
    : px_(r.px_) {
    if (nullptr != px_)
      intrusive_ptr_add_ref(px_);
  }

  template <typename U>
  IntrusivePtr(const IntrusivePtr<U>& r)
    : px_(r.get()) {
    if (nullptr != px_)
      intrusive_ptr_add_ref(px_);
  }

  IntrusivePtr(IntrusivePtr&& r)
    : px_(r.px_) {
    r.px_ = nullptr;
  }

  template <typename U>
  IntrusivePtr(IntrusivePtr<U>&& r)
    : px_(r.px_) {
    r.px_ = nullptr;
  }

  IntrusivePtr& operator=(const IntrusivePtr& r) {
    SelfType(r).swap(*this);
    return *this;
  }

  template <typename U>
  IntrusivePtr& operator=(const IntrusivePtr<U>& r) {
    SelfType(r).swap(*this);
    return *this;
  }

  IntrusivePtr& operator=(IntrusivePtr&& r) {
    SelfType(static_cast<IntrusivePtr&&>(r)).swap(*this);
    return *this;
  }

  template <typename U>
  IntrusivePtr& operator=(IntrusivePtr<U>&& r) {
    SelfType(static_cast<IntrusivePtr<U>&&>(r)).swap(*this);
    return *this;
  }

  IntrusivePtr& operator=(T* p) {
    SelfType(p).swap(*this);
    return *this;
  }

  void swap(IntrusivePtr& r) {
    std::swap(px_, r.px_);
  }

  void reset(void) {
    SelfType().swap(*this);
  }

  void reset(T* p) {
    SelfType(p).swap(*this);
  }

  void reset(T* p, bool add_ref) {
    SelfType(p, add_ref).swap(*this);
  }

  T* detach(void) {
    T* p = px_;
    px_ = nullptr;
    return p;
  }

  T* get(void) const {
    return px_;
  }

  T& operator*(void) const {
    return *px_;
  }

  T* operator->(void) const {
    return px_;
  }
};

template <typename T, typename U>
inline bool operator==(const IntrusivePtr<T>& a, const IntrusivePtr<U>& b) {
  return a.get() == b.get();
}

template <typename T, typename U>
inline bool operator!=(const IntrusivePtr<T>& a, const IntrusivePtr<U>& b) {
  return a.get() != b.get();
}

template <typename T, typename U>
inline bool operator==(const IntrusivePtr<T>& a, U* b) {
  return a.get() == b;
}

template <typename T, typename U>
inline bool operator!=(const IntrusivePtr<T>& a, U* b) {
  return a.get() != b;
}

template <typename T, typename U>
inline bool operator==(T* a, const IntrusivePtr<U>& b) {
  return a == b.get();
}

template <typename T, typename U>
inline bool operator!=(T* a, const IntrusivePtr<U>& b) {
  return a != b.get();
}

template <typename T>
inline bool operator==(const IntrusivePtr<T>& a, const IntrusivePtr<T>& b) {
  return a.get() == b.get();
}

template <typename T>
inline bool operator!=(const IntrusivePtr<T>& a, const IntrusivePtr<T>& b) {
  return a.get() != b.get();
}

template <typename T>
inline bool operator==(const IntrusivePtr<T>& p, std::nullptr_t) {
  return p.get() == nullptr;
}

template <typename T>
inline bool operator!=(const IntrusivePtr<T>& p, std::nullptr_t) {
  return p.get() != nullptr;
}

template <typename T>
inline bool operator==(std::nullptr_t, const IntrusivePtr<T>& p) {
  return nullptr == p.get();
}

template <typename T>
inline bool operator!=(std::nullptr_t, const IntrusivePtr<T>& p) {
  return nullptr != p.get();
}

template <typename T>
inline bool operator<(const IntrusivePtr<T>& a, const IntrusivePtr<T>& b) {
  return std::less<T*>()(a.get(), b.get());
}

}

#endif // CHAOS_MEMORY_INTRUSIVEPTR_H
