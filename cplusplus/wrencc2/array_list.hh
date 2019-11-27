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
#include "common.hh"
#include "container_utils.hh"

namespace wrencc {

template <typename T>
class ArrayList final : private UnCopyable {
  static constexpr sz_t kDefCapacity = 0x10;

  sz_t size_{};
  sz_t capacity_{kDefCapacity};
  T* data_{};
public:
  ArrayList(sz_t capacity = kDefCapacity) noexcept
    : capacity_(capacity < kDefCapacity ? kDefCapacity : capacity) {
    data_ = Alloc<T>::allocate(capacity_);
  }

  ~ArrayList() noexcept {
    clear();
    Alloc<T>::deallocate(data_);
  }

  ArrayList(const T& value, sz_t count) noexcept
    : size_(count)
    , capacity_(count < kDefCapacity ? kDefCapacity : count) {
    data_ = Alloc<T>::allocate(capacity_);
    fill_n(data_, count, value);
  }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline sz_t capacity() const noexcept { return capacity_; }
  inline const T* data() const noexcept { return data_; }
  inline T& at(sz_t i) noexcept { return data_[i]; }
  inline const T& at(sz_t i) const noexcept { return data_[i]; }
  inline T& operator[](sz_t i) noexcept { return data_[i]; }
  inline const T& operator[](sz_t i) const noexcept { return data_[i]; }
  inline T& first() noexcept { return data_[0]; }
  inline const T& first() const noexcept { return data_[0]; }
  inline T& last() noexcept { return data_[size_ - 1]; }
  inline const T& last() const noexcept { return data_[size_ - 1]; }

  void clear() noexcept {
    destroy(data_, data_ + size_);
  }

  void resize(sz_t size);
  void resize_capacity(sz_t capacity);
  void append(const T& x);
  void append(T&& x);
  T erase(sz_t i);

  void for_each(std::function<void (T&)>&& fn);
  void for_each(std::function<void (const T&)>&& fn);
};

}
