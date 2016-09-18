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
#ifndef __SP_SMART_PTR_HEADER_H__
#define __SP_SMART_PTR_HEADER_H__

#include "sp_shared_count.h"

namespace sp {

template <typename T> class WeakPtr;
template <typename T>
class SmartPtr {
  T* px_;
  SharedCount pn_;

  typedef SmartPtr<T> SelfType;

  template <typename Y> friend class WeakPtr;
public:
  SmartPtr(void)
    : px_(nullptr)
    , pn_() {
  }

  SmartPtr(std::nullptr_t)
    : px_(nullptr)
    , pn_() {
  }

  template <typename Y>
  explicit SmartPtr(Y* p)
    : px_(p)
    , pn_(p) {
  }

  template <typename Y, typename D>
  SmartPtr(Y* p, D d)
    : px_(p)
    , pn_(p, d) {
  }

  template <typename D>
  SmartPtr(std::nullptr_t, D d)
    : px_(nullptr)
    , pn_(nullptr, d) {
  }

  SmartPtr(const SmartPtr& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  template <typename Y>
  explicit SmartPtr(const WeakPtr<Y>& r)
    : px_(nullptr)
    , pn_(r.pn_) {
    if (!pn_.Empty())
      px_ = r.px_;
  }

  template <typename Y>
  SmartPtr(const SmartPtr<Y>& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  template <typename Y, typename D>
  SmartPtr(UniquePtr<Y, D>&& r)
    : px_(r.Get())
    , pn_() {
    pn_ = SharedCount(r);
  }

  SmartPtr(SmartPtr&& r)
    : px_(r.px_)
    , pn_() {
    pn_.Swap(r.pn_);
    r.px_ = nullptr;
  }

  template <typename Y>
  SmartPtr(SmartPtr<Y>&& r)
    : px_(r.px_)
    , pn_() {
    pn_.Swap(r.pn_);
    r.px_ = nullptr;
  }

  SmartPtr& operator=(const SmartPtr& r) {
    SelfType(r).Swap(*this);
    return *this;
  }

  template <typename Y>
  SmartPtr& operator=(const SmartPtr<Y>& r) {
    SelfType(r).Swap(*this);
    return *this;
  }

  template <typename Y, typename D>
  SmartPtr& operator=(UniquePtr<Y, D>&& r) {
    SelfType(static_cast<UniquePtr<Y, D>&&>(r)).Swap(*this);
    return *this;
  }

  SmartPtr& operator=(SmartPtr&& r) {
    SelfType(static_cast<SmartPtr&&>(r)).Swap(*this);
    return *this;
  }

  template <typename Y>
  SmartPtr& operator=(SmartPtr<Y>&& r) {
    SelfType(static_cast<SmartPtr<Y>&&>(r)).Swap(*this);
    return *this;
  }

  SmartPtr& operator=(std::nullptr_t) {
    SelfType().Swap(*this);
    return *this;
  }

  void Reset(void) {
    SelfType().Swap(*this);
  }

  template <typename Y>
  void Reset(Y* p) {
    SelfType(p).Swap(*this);
  }

  template <typename Y, typename D>
  void Reset(Y* p, D d) {
    SelfType(p, d).Swap(*this);
  }

  void Swap(SmartPtr& r) {
    std::swap(px_, r.px_);
    pn_.Swap(r.px_);
  }

  T& operator*(void) const {
    return *px_;
  }

  T* operator->(void) const {
    return px_;
  }

  T& operator[](std::ptrdiff_t i) const {
    return px_[i];
  }

  T* Get(void) const {
    return px_;
  }

  uint32_t UseCount(void) const {
    return pn_.UseCount();
  }

  bool Unique(void) const {
    return pn_.Unique();
  }
};

template <typename T, typename U>
inline bool operator==(const SmartPtr<T>& a, const SmartPtr<U>& b) {
  return a.Get() == b.Get();
}

template <typename T, typename U>
inline bool operator!=(const SmartPtr<T>& a, const SmartPtr<U>& b) {
  return a.Get() != b.Get();
}

template <typename T>
inline bool operator!=(const SmartPtr<T>& a, const SmartPtr<T>& b) {
  return a.Get() != b.Get();
}

template <typename T>
inline bool operator==(const SmartPtr<T>& p, std::nullptr_t) {
  return p.Get() == nullptr;
}

template <typename T>
inline bool operator==(std::nullptr_t, const SmartPtr<T>& p) {
  return nullptr == p.Get();
}

template <typename T>
inline bool operator!=(const SmartPtr<T>& p, std::nullptr_t) {
  return p.Get() != nullptr;
}

template <typename T>
inline bool operator!=(std::nullptr_t, const SmartPtr<T>& p) {
  return nullptr != p.Get();
}

}

#endif // __SP_SMART_PTR_HEADER_H__
