// Copyright (c) 2019 ASMlover. All rights reserved.
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

#include <functional>
#include "helper.hh"

namespace wrencc {

template <typename Tp> struct BaseArrayList : public Copyable {
  using Iterator      = Tp*;
  using ConstIterator = Tp*;
};

template <typename T>
class ArrayList final : public BaseArrayList<T> {
  static constexpr sz_t kDefCapacity = 0x10;

  sz_t size_{};
  sz_t capacity_{kDefCapacity};
  T* data_{};

  void regrow(sz_t new_capacity = 0) {
    if (new_capacity == 0)
      new_capacity = capacity_ * 3 / 2;
    if (new_capacity < kDefCapacity)
      new_capacity = kDefCapacity;

    T* new_data = SimpleAlloc<T>::allocate(new_capacity);
    try {
      uninitialized_copy(data_, data_ + size_, new_data);
    }
    catch (...) {
      destroy(new_data, new_data + size_);
      SimpleAlloc<T>::deallocate(new_data);
      throw;
    }

    destroy(data_, data_ + size_);
    SimpleAlloc<T>::deallocate(data_);

    capacity_ = new_capacity;
    data_ = new_data;
  }
public:
  ArrayList(sz_t capacity = kDefCapacity) noexcept
    : capacity_(capacity < kDefCapacity ? kDefCapacity : capacity) {
    data_ = SimpleAlloc<T>::allocate(capacity_);
  }

  ~ArrayList() noexcept {
    clear();
    SimpleAlloc<T>::deallocate(data_);
  }

  ArrayList(sz_t count, const T& value) noexcept
    : size_(count)
    , capacity_(count < kDefCapacity ? kDefCapacity : count) {
    data_ = SimpleAlloc<T>::allocate(capacity_);
    uninitialized_fill(data_, size_, value);
  }

  ArrayList(const ArrayList<T>& r) noexcept
    : size_(r.size_)
    , capacity_(r.capacity_) {
    data_ = SimpleAlloc<T>::allocate(capacity_);
    uninitialized_copy(r.begin(), r.end(), begin());
  }

  ArrayList(ArrayList<T>&& r) noexcept {
    r.swap(*this);
  }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline sz_t capacity() const noexcept { return capacity_; }
  inline T* data() noexcept { return data_; }
  inline const T* data() const noexcept { return data_; }
  inline T& at(sz_t i) noexcept { return data_[i]; }
  inline const T& at(sz_t i) const noexcept { return data_[i]; }
  inline T& operator[](sz_t i) noexcept { return data_[i]; }
  inline const T& operator[](sz_t i) const noexcept { return data_[i]; }
  inline T& get_head() noexcept { return *begin(); }
  inline const T& get_head() const noexcept { return *begin(); }
  inline T& get_tail() noexcept { return *(end() - 1); }
  inline const T& get_tail() const noexcept { return *(end() - 1); }
  inline Iterator begin() noexcept { return data_; }
  inline ConstIterator begin() const noexcept { return data_; }
  inline Iterator end() noexcept { return data_ + size_; }
  inline ConstIterator end() const noexcept { return data_ + size_; }

  inline void clear() noexcept {
    destroy(begin(), end());
    size_ = 0;
  }

  template <typename Function> void for_each(Function&& fn) noexcept {
    for (sz_t i = 0; i < size_; ++i)
      fn(data_[i]);
  }

  void swap(ArrayList<T>& r) noexcept {
    std::swap(size_, r.size_);
    std::swap(capacity_, r.capacity_);
    std::swap(data_, r.data_);
  }

  void resize(sz_t size) {
    if (size > size_) {
      regrow(size * 3 / 2);
    }
    else if (size < size_) {
      destroy(begin() + size, end());
      size_ = size;
    }
  }

  void append(const T& x) {
    if (size_ >= capacity_)
      regrow();
    construct(&data_[size_++], x);
  }

  void append(T&& x) {
    if (size_ >= capacity_)
      regrow();
    construct(&data_[size_++], std::move(x));
  }

  template <typename... Args> void append(Args&&... args) {
    if (size_ >= capacity_)
      regrow();
    construct(&data_[size_++], std::forward<Args>(args)...);
  }

  T pop() {
    T r = data_[size_ - 1];
    destroy(&data_[--size_]);
    return r;
  }
};

}
