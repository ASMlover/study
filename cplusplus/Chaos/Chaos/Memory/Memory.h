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
#ifndef CHAOS_MEMORY_MEMORY_H
#define CHAOS_MEMORY_MEMORY_H

#include <Chaos/Memory/SharedCount.h>

namespace Chaos {

template <typename T>
inline void checked_delete(T* p) {
  static_assert(sizeof(T), "`T` must be a valid type");
  delete p;
}

template <typename T>
inline void checked_array_delete(T* p) {
  static_assert(sizeof(T), "`T` must be a valid type");
  delete [] p;
}

template <typename T>
struct CheckedDeleter : private UnCopyable {
  void operator()(T* p) {
    checked_delete(p);
  }
};

template <typename T>
struct CheckedArrayDeleter : private UnCopyable {
  void operator()(T* p) {
    checked_array_delete(p);
  }
};

template <typename T>
class ScopedPtr : private UnCopyable {
  T* px_{};

  typedef ScopedPtr<T> SelfType;
public:
  ScopedPtr(void) = default;

  explicit ScopedPtr(T* p = nullptr)
    : px_(p) {
  }

  ScopedPtr(std::nullptr_t)
    : px_(nullptr) {
  }

  ~ScopedPtr(void) {
    if (nullptr != px_)
      delete px_;
  }

  ScopedPtr(ScopedPtr&& r)
    : px_(r.px_) {
    r.px_ = nullptr;
  }

  ScopedPtr& operator=(std::nullptr_t) {
    reset();
    return *this;
  }

  ScopedPtr& operator=(ScopedPtr&& r) {
    if (&r != this) {
      if (nullptr != px_)
        delete px_;
      px_ = r.px_;
      r.px_ = nullptr;
    }
    return *this;
  }

  T& operator*(void) const {
    return *px_;
  }

  T* operator->(void) const {
    return px_;
  }

  T* get(void) const {
    return px_;
  }

  explicit operator bool(void) const {
    return nullptr != px_;
  }

  void reset(T* p = nullptr) {
    SelfType(p).swap(*this);
  }

  void swap(ScopedPtr& r) {
    std::swap(px_, r.px_);
  }
};

template <typename T>
inline bool operator==(const ScopedPtr<T>& p, std::nullptr_t) {
  return p.get() == nullptr;
}

template <typename T>
inline bool operator!=(const ScopedPtr<T>& p, std::nullptr_t) {
  return p.get() != nullptr;
}

template <typename T>
inline bool operator==(std::nullptr_t, const ScopedPtr<T>& p) {
  return nullptr == p.get();
}

template <typename T>
inline bool operator!=(std::nullptr_t, const ScopedPtr<T>& p) {
  return nullptr != p.get();
}

template <typename T>
class ScopedArray : private UnCopyable {
  T* px_{};

  typedef ScopedArray<T> SelfType;
public:
  ScopedArray(void) = default;

  ScopedArray(std::nullptr_t)
    : px_(nullptr) {
  }

  explicit ScopedArray(T* p = nullptr)
    : px_(p) {
  }

  ~ScopedArray(void) {
    if (nullptr != px_)
      delete [] px_;
  }

  ScopedArray(ScopedArray&& r)
    : px_(r.px_) {
    r.px_ = nullptr;
  }

  ScopedArray& operator=(std::nullptr_t) {
    reset();
    return *this;
  }

  ScopedArray& operator=(ScopedArray&& r) {
    if (&r != this) {
      if (nullptr != px_)
        delete [] px_;
      px_ = r.px_;
      r.px_ = nullptr;
    }
    return *this;
  }

  T& operator[](std::ptrdiff_t i) const {
    return px_[i];
  }

  T* get(void) const {
    return px_;
  }

  explicit operator bool(void) const {
    return nullptr != px_;
  }

  void reset(T* p = nullptr) {
    SelfType(p).swap(*this)
  }

  void swap(ScopedArray& r) {
    std::swap(px_, r.px_);
  }
};

template <typename T>
inline bool operator==(const ScopedArray<T>& p, std::nullptr_t) {
  return p.get() == nullptr;
}

template <typename T>
inline bool operator!=(const ScopedArray<T>& p, std::nullptr_t) {
  return p.get() != nullptr;
}

template <typename T>
inline bool operator==(std::nullptr_t, const ScopedArray<T>& p) {
  return nullptr == p.get();
}

template <typename T>
inline bool operator!=(std::nullptr_t, const ScopedArray<T>& p) {
  return nullptr != p.get();
}

template <typename T> class WeakPtr;

template <typename T>
class SharedPtr {
  T* px_{};
  SharedCount pn_{};

  template <typename Y> friend class SharedPtr;
  template <typename Y> friend class WeakPtr;
  typedef SharedPtr<T> SelfType;
public:
  SharedPtr(void) = default;
  ~SharedPtr(void) = default;

  template <typename Y>
  explicit SharedPtr(Y* p)
    : px_(p)
    , pn_(px_) {
  }

  template <typename Y, typename D>
  SharedPtr(Y* p, D& d)
    : px_(p)
    , pn_(px_, d) {
  }

  template <typename D>
  SharedPtr(std::nullptr_t, D& d)
    : px_(nullptr)
    , pn_(px_, d) {
  }

  template <typename Y>
  explicit SharedPtr(const WeakPtr<Y>& r)
    : pn_(r.pn_) {
    if (!pn_.empty())
      px_ = r.px_;
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
  void reset(Y* p, D& d) {
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

  T& operator[](std::ptrdiff_t i) const {
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
  WeakCount pn_{};

  template <typename Y> friend class WeakPtr;
  template <typename Y> friend class SharedPtr;
  typedef WeakPtr<T> SelfType;
public:
  WeakPtr(void) = default;
  ~WeakPtr(void) = default;

  template <typename Y>
  explicit WeakPtr(const SharedPtr<Y>& r)
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
    SelfType(static_cast<WeakPtr&&>(r)).swap(*this);
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

template <typename T>
class SharedArray {
  T* px_{};
  SharedCount pn_{};

  template <typename Y> friend class SharedArray;
  typedef SharedArray<T> SelfType;
public:
  SharedArray(void) = default;
  ~SharedArray(void) = default;

  SharedArray(std::nullptr_t) {
  }

  template <typename Y>
  SharedArray(Y* p)
    : px_(p)
    , pn_(p, CheckedArrayDeleter<Y>(p)) {
  }

  template <typename Y, typename D>
  SharedArray(Y* p, D& d)
    : px_(p)
    , pn_(p, d) {
  }

  SharedArray(const SharedArray& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  template <typename Y>
  SharedArray(const SharedArray<Y>& r)
    : px_(r.px_)
    , pn_(r.pn_) {
  }

  SharedArray(SharedArray&& r)
    : px_(r.px_)
    , pn_() {
    pn_.swap(r.pn_);
    r.px_ = nullptr;
  }

  template <typename Y>
  SharedArray(SharedArray<Y>&& r)
    : px_(r.px_)
    , pn_() {
    pn_.swap(r.pn_);
    r.px_ = nullptr;
  }

  SharedArray& operator=(const SharedArray& r) {
    SelfType(r).swap(*this);
    return *this;
  }

  template <typename Y>
  SharedArray& operator=(const SharedArray<Y>& r) {
    SelfType(r).swap(*this);
    return *this;
  }

  SharedArray& operator=(SharedArray&& r) {
    SelfType(static_cast<SharedArray&&>(r)).swap(*this);
    return *this;
  }

  template <typename Y>
  SharedArray& operator=(SharedArray<Y>&& r) {
    SelfType(static_cast<SharedArray<Y>&&>(r)).swap(*this);
    return *this;
  }

  void swap(SharedArray& r) {
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
  void reset(Y* p, D& d) {
    SelfType(p, d).swap(*this);
  }

  template <typename Y>
  void reset(const SharedArray<Y>& r) {
    SelfType(r).swap(*this);
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

  T& operator[](std::ptrdiff_t i) const {
    return px_[i];
  }

  T* get(void) const {
    return px_;
  }
};

template <typename T, typename U>
inline bool operator==(const SharedArray<T>& a, const SharedArray<U>& b) {
  return a.get() == b.get();
}

template <typename T, typename U>
inline bool operator!=(const SharedArray<T>& a, const SharedArray<U>& b) {
  return a.get() != b.get();
}

template <typename T>
inline bool operator!=(const SharedArray<T>& a, const SharedArray<T>& b) {
  return a.get() != b.get();
}

template <typename T>
inline bool operator==(const SharedArray<T>& p, std::nullptr_t) {
  return p.get() == nullptr;
}

template <typename T>
inline bool operator!=(const SharedArray<T>& p, std::nullptr_t) {
  return p.get() != nullptr;
}

template <typename T>
inline bool operator==(std::nullptr_t, const SharedArray<T>& p) {
  return nullptr == p.get();
}

template <typename T>
inline bool operator!=(std::nullptr_t, const SharedArray<T>& p) {
  return nullptr != p.get();
}

}

#endif // CHAOS_MEMORY_MEMORY_H
