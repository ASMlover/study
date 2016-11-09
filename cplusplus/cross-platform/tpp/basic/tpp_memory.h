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
#ifndef TPP_BASIC_MEMORY_H_
#define TPP_BASIC_MEMORY_H_

#include "tpp_counted.h"

namespace tpp { namespace basic {

template <typename T> class WeakPtr;
template <typename T>
class SharedPtr {
  T* px_{};
  SharedCount pn_;

  template <typename Y> class WeakPtr;
  typedef SharedPtr<T> SelfType;
public:
  SharedPtr(void) = default;
  SharedPtr(std::nullptr_t) = default;
  ~SharedPtr(void) = default;

  template <typename Y>
  explicit SharedPtr(Y* p)
    : px_(p)
    , pn_(px_) {
  }

  template <typename Y, typename D>
  SharedPtr(Y* p, D d)
    : px_(p)
    , pn_(p, d) {
  }

  template <typename D>
  SharedPtr(std::nullptr_t, D d)
    : px_(nullptr)
    , pn_(nullptr, d) {
  }

  SharedPtr(const SharedPtr& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  SharedPtr(SharedPtr&& r)
    : px_(r.px_)
    , pn_() {
    pn_.swap(r.pn_);
    r.px_ = nullptr;
  }

  template <typename Y>
  SharedPtr(const SharedPtr<Y>& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  template <typename Y>
  SharedPtr(SharedPtr<Y>&& r)
    : px_(r.px_)
    , pn_() {
    pn_.swap(r.pn_);
    r.px_ = nullptr;
  }

  SharedPtr& operator=(const SharedPtr& r) {
    SelfType(r).swap(*this);
    return *this;
  }

  SharedPtr& operator=(SharedPtr&& r) {
    SelfType(static_cast<SharedPtr&&>(r)).swap(*this);
    return *this;
  }

  template <typename Y>
  SharedPtr& operator=(const SharedPtr<Y>& r) {
    SelfType(r).swap(*this);
    return *this;
  }

  template <typename Y>
  SharedPtr& operator=(SharedPtr<Y>&& r) {
    SelfType(static_cast<SharedPtr<Y>&&>(r)).swap(*this);
    return *this;
  }

  SharedPtr& operator=(std::nullptr_t) {
    SelfType().swap(*this);
    return *this;
  }

  void swap(SharedPtr& r) {
    std::swap(px_, r.px_);
    pn_.swap(r.pn_);
  }

  void reset(void) {
    SelfType().swap(*this);
  }

  template <typename Y>
  void reset(Y* p) {
    SelfType(p).swap(*this);
  }

  template <typename Y, typename D>
  void reset(Y* p, D d) {
    SelfType(p, d).swap(*this);
  }

  template <typename Y>
  void reset(const SharedPtr<Y>& r) {
    SelfType(r).swap(*this);
  }

  template <typename Y>
  void reset(SharedPtr<Y>&& r) {
    SelfType(static_cast<SharedPtr<Y>&&>(r)).swap(*this);
  }

  T& operator*(void) const {
    return *px_;
  }

  T* operator->(void) const {
    return px_;
  }

  T& operator[](size_t i) const {
    return px_[i];
  }

  uint32_t use_count(void) const {
    return pn_.use_count();
  }

  bool unique(void) const {
    return pn_.unique();
  }

  explicit operator bool(void) const {
    return nullptr != px_;
  }

  T* get(void) const {
    return px_;
  }

  template <typename Y>
  bool less(const SharedPtr<Y>& r) const {
    return pn_ < r.pn_;
  }

  template <typename Y>
  bool less(const WeakPtr<Y>& r) const {
    return pn_ < r.pn_;
  }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& a, const SharedPtr<U>& b) {
  return a.get() == b.get();
}

template <typename T, typename U>
inline bool operator!=(const SharedPtr<T>& a, const SharedPtr<U>& b) {
  return a.get() != b.get();
}

template <typename T, typename U>
inline bool operator<(const SharedPtr<T>& a, const SharedPtr<U>& b) {
  return a.less(b);
}

template <typename T>
inline bool operator!=(const SharedPtr<T>& a, const SharedPtr<T>& b) {
  return a.get() != b.get();
}

template <typename T>
inline bool operator==(const SharedPtr<T>& p, std::nullptr_t) {
  return p.get() == nullptr;
}

template <typename T>
inline bool operator!=(const SharedPtr<T>& p, std::nullptr_t) {
  return p.get() != nullptr;
}

template <typename T>
inline bool operator==(std::nullptr_t, const SharedPtr<T>& p) {
  return nullptr == p.get();
}

template <typename T>
inline bool operator!=(std::nullptr_t, const SharedPtr<T>& p) {
  return nullptr != p.get();
}

template <typename T, typename... Args>
inline SharedPtr<T> make_shared(Args&&... args) {
  return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
class WeakPtr {
  T* px_{};
  WeakCount pn_;

  template <typename Y> friend class SharedPtr;
  typedef WeakPtr<T> SelfType;
public:
  WeakPtr(void) = default;

  template <typename Y>
  WeakPtr(const SharedPtr<Y>& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  WeakPtr(const WeakPtr& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  template <typename Y>
  WeakPtr(const WeakPtr<Y>& r)
    : px_(r.lock().get())
    , pn_(r.pn_) {
  }

  WeakPtr(WeakPtr&& r)
    : px_(r.px_)
    , pn_(std::move(r.pn_)) {
    r.px_ = nullptr;
  }

  template <typename Y>
  WeakPtr(WeakPtr<Y>&& r)
    : px_(r.px_)
    , pn_(std::move(r.pn_)) {
    r.px_ = nullptr;
  }

  WeakPtr& operator=(const WeakPtr& r) {
    px_ = r.px_;
    pn_ = r.pn_;
    return *this;
  }

  template <typename Y>
  WeakPtr& operator=(const WeakPtr<Y>& r) {
    px_ = r.lock().get();
    pn_ = r.pn_;
    return *this;
  }

  WeakPtr& operator=(WeakPtr&& r) {
    SelfType(static_cast<WeakPtr>(r)).swap(*this);
    return *this;
  }

  template <typename Y>
  WeakPtr& operator=(WeakPtr<Y>&& r) {
    SelfType(static_cast<WeakPtr<Y>&&>(r)).swap(*this);
    return *this;
  }

  template <typename Y>
  WeakPtr& operator=(const SharedPtr<Y>& r) {
    px_ = r.px_;
    pn_ = r.pn_;
    return *this;
  }

  SharedPtr<T> lock(void) const {
    return SharedPtr<T>(*this);
  }

  void reset(void) {
    SelfType().swap(*this);
    return *this;
  }

  void swap(WeakPtr& r) {
    std::swap(px_, r.px_);
    pn_.swap(r.pn_);
  }

  uint32_t use_count(void) const {
    return pn_.use_count();
  }

  bool expired(void) const {
    return 0 == use_count();
  }

  template <typename Y>
  bool less(const WeakPtr<Y>& r) const {
    return pn_ < r.pn_;
  }

  template <typename Y>
  bool less(const SharedPtr<Y>& r) const {
    return pn_ < r.pn_;
  }
};

template <typename T, typename U>
inline bool operator<(const WeakPtr<T>& a, const WeakPtr<U>& b) {
  return a.less(b);
}

}}

#endif // TPP_BASIC_MEMORY_H_
