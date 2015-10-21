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
#ifndef __TYR_UNIQUE_ARRAY_HEADER_H__
#define __TYR_UNIQUE_ARRAY_HEADER_H__

namespace tyr {

template <typename T>
class UniqueArray : private UnCopyable {
  T* ptr_;

  typedef UniqueArray<T> UniqueArrayT;
public:
  explicit UniqueArray(T* p = nullptr) tyr_noexcept
    : ptr_(p) {
  }

  ~UniqueArray(void) {
    if (nullptr != ptr_)
      delete [] ptr_;
  }

  UniqueArray(UniqueArrayT&& other) tyr_noexcept
    : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }

  UniqueArrayT& operator=(UniqueArrayT&& other) tyr_noexcept {
    if (&other != this) {
      if (nullptr != ptr_)
        delete [] ptr_;
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    }
    return *this;
  }

  void Reset(T* p = nullptr) tyr_noexcept {
    UniqueArrayT(p).Swap(*this);
  }

  void Swap(UniqueArrayT& other) tyr_noexcept {
    std::swap(ptr_, other.ptr_);
  }

  T* Get(void) const tyr_noexcept {
    return ptr_;
  }

  T& operator[](std::ptrdiff_t i) const {
    TYR_ASSERT(nullptr != ptr_ && i >= 0);
    return ptr_[i];
  }

  explicit operator bool(void) const tyr_noexcept {
    return nullptr != ptr_;
  }
};

template <typename T>
bool operator==(const UniqueArray<T>& p, nullptr_t) tyr_noexcept {
  return p.Get() == nullptr;
}

template <typename T>
bool operator==(nullptr_t, const UniqueArray<T>& p) tyr_noexcept {
  return nullptr == p.Get();
}

template <typename T>
bool operator!=(const UniqueArray<T>& p, nullptr_t) tyr_noexcept {
  return p.Get() != nullptr;
}

template <typename T>
bool operator!=(nullptr_t, const UniqueArray<T>& p) tyr_noexcept {
  return nullptr != p.Get();
}

template <typename T>
std::ostream& operator<<(std::ostream& cout, const UniqueArray<T>& p) {
  return cout << p.Get();
}

template <typename T>
T* GetPointer(const UniqueArray<T>& p) tyr_noexcept {
  return p.Get();
}

template <typename T>
void Swap(UniqueArray<T>& x, UniqueArray<T>& y) tyr_noexcept {
  x.Swap(y);
}

}

#endif  // __TYR_UNIQUE_ARRAY_HEADER_H__
