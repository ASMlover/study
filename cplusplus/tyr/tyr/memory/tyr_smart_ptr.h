// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __TYR_SMART_PTR_HEADER_H__
#define __TYR_SMART_PTR_HEADER_H__

namespace tyr {

class RefPointer : private UnCopyable {
public:
  virtual ~RefPointer(void) tyr_noexcept {}
  virtual void Destroy(void) = 0;
};

template <typename T>
class RefPtrDelete : public RefPointer {
  T* ptr_;
public:
  explicit RefPtrDelete(T* p) tyr_noexcept
    : ptr_(p) {
  }

  virtual void Destroy(void) override {
    if (nullptr != ptr_)
      delete ptr_;
  }
};

template <typename T, typename D>
class RefPtrDestructor : public RefPointer {
  T* ptr_;
  D  dtor_;
public:
  RefPtrDestructor(T* p, D d) tyr_noexcept
    : ptr_(p)
    , dtor_(d) {
  }

  virtual void Destroy(void) override {
    if (nullptr != ptr_)
      dtor_(ptr_);
  }
};

// SmartPtr
template <typename T>
class SmartPtr {
  T*             ptr_;
  RefPointer*    rp_;
  AtomicCounter* rc_;

  typedef SmartPtr<T> SelfType;
public:
  SmartPtr(void) tyr_noexcept
    : ptr_(nullptr)
    , rp_(nullptr)
    , rc_(nullptr) {
  }

  SmartPtr(nullptr_t) tyr_noexcept
    : ptr_(nullptr)
    , rp_(nullptr)
    , rc_(nullptr) {
  }

  template <typename U>
  explicit SmartPtr(U* p)
    : ptr_(p)
    , rp_(new RefPtrDelete<T>(ptr_))
    , rc_(new AtomicCounter(1)) {
  }

  template <typename U, typename D>
  SmartPtr(U* p, D d)
    : ptr_(p)
    , rp_(new RefPtrDestructor<T, D>(ptr_, d))
    , rc_(new AtomicCounter(1)) {
  }

  ~SmartPtr(void) {
    if (nullptr != rc_ && 0 == --*rc_) {
      rp_->Destroy();
      ptr_ = nullptr;

      delete rp_;
      rp_ = nullptr;

      delete rc_;
      rc_ = nullptr;
    }
  }

  SmartPtr(const SmartPtr<T>& other) tyr_noexcept
    : ptr_(other.ptr_)
    , rp_(other.rp_)
    , rc_(other.rc_) {
    if (nullptr != rc_)
      ++*rc_;
  }

  SmartPtr(SmartPtr<T>&& other) tyr_noexcept
    : ptr_(other.ptr_)
    , rp_(other.rp_)
    , rc_(other.rc_) {
    other.__restore_defaults__();
  }

  template <typename U>
  SmartPtr(const SmartPtr<U>& other) tyr_noexcept
    : ptr_(other.Get())
    , rp_(other.__rp__())
    , rc_(other.__rc__()) {
    if (nullptr != rc_)
      ++*rc_;
  }

  template <typename U>
  SmartPtr(SmartPtr<U>&& other) tyr_noexcept
    : ptr_(other.Get())
    , rp_(other.__rp__())
    , rc_(other.__rc__()) {
    other.__restore_defaults__();
  }

  SmartPtr<T>& operator=(const SmartPtr<T>& other) tyr_noexcept {
    if (&other != this)
      SelfType(other).Swap(*this);
    return *this;
  }

  SmartPtr<T>& operator=(SmartPtr<T>&& other) tyr_noexcept {
    if (&other != this)
      SelfType(std::move(other)).Swap(*this);
    return *this;
  }

  template <typename U>
  SmartPtr<T>& operator=(const SmartPtr<U>& other) tyr_noexcept {
    if ((void*)&other != (void*)this)
      SelfType(other).Swap(*this);
    return *this;
  }

  template <typename U>
  SmartPtr<T>& operator=(SmartPtr<U>&& other) tyr_noexcept {
    if ((void*)&other != (void*)this)
      SelfType(std::move(other)).Swap(*this);
    return *this;
  }

  T* Get(void) const tyr_noexcept {
    return ptr_;
  }

  void Swap(SmartPtr<T>& other) tyr_noexcept {
    std::swap(ptr_, other.ptr_);
    std::swap(rp_, other.rp_);
    std::swap(rc_, other.rc_);
  }

  void Reset(void) tyr_noexcept {
    SelfType().Swap(*this);
  }

  template <typename U>
  void Reset(U* p) {
    SelfType(p).Swap(*this);
  }

  template <typename U, typename D>
  void Reset(U* p, D d) {
    SelfType(p, d).Swap(p, d);
  }

  T& operator*(void) const {
    return *ptr_;
  }

  T* operator->(void) const tyr_noexcept {
    return ptr_;
  }

  explicit operator bool(void) const tyr_noexcept {
    return nullptr != ptr_;
  }
public:
  RefPointer* __rp__(void) const tyr_noexcept {
    return rp_;
  }

  AtomicCounter* __rc__(void) const tyr_noexcept {
    return rc_;
  }

  void __restore_defaults__(void) tyr_noexcept {
    ptr_ = nullptr;
    rp_  = nullptr;
    rc_  = nullptr;
  }
};

template <typename T, typename U>
bool operator==(const SmartPtr<T>& x, const SmartPtr<U>& y) tyr_noexcept {
  return x.Get() == y.Get();
}

template <typename T>
bool operator==(const SmartPtr<T>& p, nullptr_t) tyr_noexcept {
  return p.Get() == nullptr;
}

typename <typename T>
bool operator==(nullptr_t, const SmartPtr<T>& p) tyr_noexcept {
  return nullptr == p.Get();
}

template <typename T, typename U>
bool operator!=(const SmartPtr<T>& x, const SmartPtr<T>& y) tyr_noexcept {
  return x.Get() != y.Get();
}

template <typename T>
bool operator!=(const SmartPtr<T>& p, nullptr_t) tyr_noexcept {
  return p.Get() != nullptr;
}

template <typename T>
bool operator!=(nullptr_t, const SmartPtr<T>& p) tyr_noexcept {
  return nullptr != p.Get();
}

template <typename T, typename U>
bool operator<(const SmartPtr<T>& x, const SmartPtr<U>& y) tyr_noexcept {
  return x.Get() < y.Get();
}

template <T>
std::ostream& operator<<(std::ostream& cout, const SmartPtr<T>& p) {
  return cout << p.Get();
}

template <T>
T* GetPointer(const SmartPtr<T>& p) tyr_noexcept {
  return p.Get();
}

template <typename T>
void Swap(SmartPtr<T>& x, SmartPtr<U>& y) tyr_noexcept {
  x.Swap(y);
}

}

#endif  // __TYR_SMART_PTR_HEADER_H__
