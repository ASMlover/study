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
#ifndef __TYR_SMART_ARRAY_HEADER_H__
#define __TYR_SMART_ARRAY_HEADER_H__

namespace tyr {

class RefArray : private UnCopyable {
public:
  virtual ~RefArray(void) tyr_noexcept {}
  virtual void Destroy(void) = 0;
};

template <typename T>
class RefArrayDelete : public RefArray {
  T* ptr_;
public:
  explicit RefArrayDelete(T* p) tyr_noexcept
    : ptr_(p) {
  }

  virtual void Destroy(void) override {
    if (nullptr != ptr_)
      delete [] ptr_;
  }
};

template <typename T, typename D>
class RefArrayDestructor : public RefArray {
  T* ptr_;
  D  dtor_;
public:
  RefArrayDestructor(T* p, D d) tyr_noexcept
    : ptr_(p)
    , dtor_(d) {
  }

  virtual void Destroy(void) override {
    if (nullptr != ptr_)
      dtor_(ptr_);
  }
};

// SmartArray
template <typename T>
class SmartArray {
  T*             ptr_;
  RefArray*      ra_;
  AtomicCounter* rc_;

  typedef SmartArray<T> SelfType;
public:
  SmartArray(void) tyr_noexcept
    : ptr_(nullptr)
    , ra_(nullptr)
    , rc_(nullptr) {
  }

  template <typename U>
  explicit SmartArray(U* p)
    : ptr_(p)
    , ra_(new RefArrayDelete<T>(ptr_))
    , rc_(new AtomicCounter(1)) {
  }

  template <typename U, typename D>
  SmartArray(U* p, D d)
    : ptr_(p)
    , ra_(new RefArrayDestructor<T, D>(ptr_, d))
    , rc_(AtomicCounter(1)) {
  }

  ~SmartArray(void) {
    if (nullptr != rc_ && 0 == --*rc_) {
      ra_->Destroy();
      ptr_ = nullptr;

      delete ra_;
      ra_ = nullptr;

      delete rc_;
      rc_ = nullptr;
    }
  }

  SmartArray(const SmartArray& other) tyr_noexcept
    : ptr_(other.ptr_)
    , ra_(other.ra_)
    , rc_(other.rc_) {
    if (nullptr != rc_)
      ++*rc_;
  }

  SmartArray(SmartArray&& other) tyr_noexcept
    : ptr_(other.ptr_)
    , ra_(other.ra_)
    , rc_(other.rc_) {
    other.__restore_defaults__();
  }

  template <typename U>
  SmartArray(const SmartArray<U>& other) tyr_noexcept
    : ptr_(other.Get())
    , ra_(other.__ra__())
    , rc_(other.__rc__()) {
    if (nullptr != rc_)
      ++*rc_;
  }

  template <typename U>
  SmartArray(SmartArray<U>&& other) tyr_noexcept
    : ptr_(other.Get())
    , ra_(other.__ra__())
    , rc_(other.__rc__()) {
    other.__restore_defaults__();
  }

  SmartArray& operator=(const SmartArray& other) tyr_noexcept {
    if (&other != this)
      SelfType(other).Swap(*this);
    return *this;
  }

  SmartArray& operator=(SmartArray&& other) tyr_noexcept {
    if (&other != this)
      SelfType(std::move(other)).Swap(*this);
    return *this;
  }

  template <typename U>
  SmartArray& operator=(const SmartArray<U>& other) tyr_noexcept {
    if ((void*)&other != (void*)this)
      SelfType(other).Swap(*this);
    return *this;
  }

  template <typename U>
  SmartArray& operator=(SmartArray<U>&& other) tyr_noexcept {
    if ((void*)&other != (void*)this)
      SelfType(std::move(other)).Swap(*this);
    return *this;
  }

  T* Get(void) const tyr_noexcept {
    return ptr_;
  }

  void Swap(SmartArray<T>& other) tyr_noexcept {
    std::swap(ptr_, other.ptr_);
    std::swap(ra_, other.ra_);
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
    SelfType(p, d).Swap(*this);
  }

  T& operator[](std::ptrdiff_t i) const {
    TYR_ASSERT(nullptr != ptr_ && i >= 0);
    return ptr_[i];
  }

  explicit operator bool(void) const tyr_noexcept {
    return nullptr != ptr_;
  }
public:
  RefArray* __ra__(void) const tyr_noexcept {
    return ra_;
  }

  AtomicCounter* __rc__(void) const tyr_noexcept {
    return rc_;
  }

  void __restore_defaults__(void) tyr_noexcept {
    ptr_ = nullptr;
    ra_  = nullptr;
    rc_  = nullptr;
  }
};

template <typename T, typename U>
bool operator==(const SmartArray<T>& x, const SmartArray<U>&) tyr_noexcept {
  return x.Get() == y.Get();
}

template <typename T>
bool operator==(const SmartArray<T>& p, nullptr_t) tyr_noexcept {
  return p.Get() == nullptr;
}

template <typename T>
bool operator==(nullptr_t, const SmartArray<T>& p) tyr_noexcept {
  return nullptr == p.Get();
}

template <typename T, typename U>
bool operator!=(const SmartArray<T>& x, const SmartArray<U>& y) tyr_noexcept {
  return x.Get() != y.Get();
}

template <typename T>
bool operator!=(const SmartArray<T>& p, nullptr_t) tyr_noexcept {
  return p.Get() == nullptr;
}

template <typename T>
bool operator!=(nullptr_t, const SmartArray<T>& p) tyr_noexcept {
  return nullptr != p.Get();
}

template <typename T, typename U>
bool operator<(const SmartArray<T>& x, const SmartArray<U>& y) tyr_noexcept {
  return x.Get() < y.Get();
}

template <typename T>
std::ostream& operator<<(std::ostream& cout, const SmartArray<T>& p) {
  return cout << p.Get();
}

template <typename T>
T* GetPointer(const SmartArray<T>& p) tyr_noexcept {
  return p.Get();
}

template <typename T>
void Swap(SmartArray<T>& x, SmartArray<T>& y) tyr_noexcept {
  x.Swap(y);
}

}

#endif  // __TYR_SMART_ARRAY_HEADER_H__
