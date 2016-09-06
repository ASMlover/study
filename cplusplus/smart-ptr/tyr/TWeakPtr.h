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
#ifndef __TYR_WEAK_PTR_HEADER_H__
#define __TYR_WEAK_PTR_HEADER_H__

#include "TSharedCount.h"
#include "TSmartPtr.h"

namespace tyr {

template <typename T> class SmartPtr;

template <typename T>
class WeakPtr {
  T* px_;
  WeakCount pn_;

  typedef WeakPtr<T> SelfType;
  template <typename Y> friend class SmartPtr;
public:
  WeakPtr(void)
    : px_(nullptr)
    , pn_() {
  }

  WeakPtr(const WeakPtr& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  WeakPtr& operator=(const WeakPtr& r) {
    px_ = r.px_;
    pn_ = r.pn_;

    return *this;
  }

  template <typename Y>
  WeakPtr& operator=(const WeakPtr<Y>& r) {
    px_ = r.Lock().Get();
    pn_ = r.pn_;

    return *this;
  }

  template <typename Y>
  WeakPtr(const WeakPtr<Y>& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  WeakPtr(WeakPtr&& r)
    : px_(r.px_)
    , pn_(static_cast<WeakCount&&>(r.pn_)) {
  }

  template <typename Y>
  WeakPtr(WeakPtr<Y>&& r)
    : px_(r.Lock().Get())
    , pn_(static_cast<WeakCount&&>(r.pn_)) {
  }

  WeakPtr& operator=(WeakPtr&& r) {
    SelfType(static_cast<WeakPtr&&>(r)).Swap(*this);
    return *this;
  }

  template <typename Y>
  WeakPtr& operator=(WeakPtr<Y>&& r) {
    SelfType(static_cast<WeakPtr<Y>&&>(r)).Swap(*this);
    return *this;
  }

  template <typename Y>
  WeakPtr(const SmartPtr<Y>& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  template <typename Y>
  WeakPtr& operator=(const SmartPtr<Y>& r) {
    px_ = r.px_;
    pn_ = r.pn_;

    return *this;
  }

  SmartPtr<T> Lock(void) const {
    return SmartPtr<T>(*this);
  }

  uint32_t UseCount(void) const {
    return pn_.UseCount();
  }

  bool Expired(void) const {
    return pn_.UseCount() == 0;
  }

  void Reset(void) {
    SelfType().Swap(*this);
  }

  void Swap(WeakPtr& r) {
    std::swap(px_, r.px_);
    pn_.Swap(r.pn_);
  }
};

template <typename T, typename U>
inline bool operator<(const WeakPtr<T>& a, const WeakPtr<U>& b) {
  return a.pn_ < b.pn_;
}

template <typename T>
inline void Swap(WeakPtr<T>& a, WeakPtr<T>& b) {
  a.Swap(b);
}

}

#endif // __TYR_WEAK_PTR_HEADER_H__
