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

template <typename Tp> class ArrayList final : public Copyable {
public:
  using ValueType = Tp;
  using Iter      = Tp*;
  using ConstIter = const Tp*;
  using Ptr       = Tp*;
  using ConstPtr  = const Tp*;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
private:
  using Alloc     = SimpleAlloc<ValueType>;

  static constexpr sz_t kDefCapacity = 0x10;

  sz_t size_{};
  sz_t capacity_{kDefCapacity};
  Ptr data_{};

  void regrow(sz_t new_capacity = 0) {
    if (new_capacity == 0)
      new_capacity = capacity_ * 3 / 2;
    if (new_capacity < kDefCapacity)
      new_capacity = kDefCapacity;

    Ptr new_data = Alloc::allocate(new_capacity);
    try {
      uninitialized_copy(begin(), end(), new_data);
    }
    catch (...) {
      destroy(new_data, new_data + size_);
      Alloc::deallocate(new_data);
      throw;
    }

    destroy(begin(), end());
    Alloc::deallocate(data_);

    capacity_ = new_capacity;
    data_ = new_data;
  }

  void destructor_aux() noexcept {
    clear();
    Alloc::deallocate(data_);
  }

  void insert_aux(ConstIter pos, const ValueType& val) {
    Ptr p = data_ + (pos - begin());
    if (size_ < capacity_) {
      if (p == data_ + size_) {
        construct(data_ + size_++, val);
      }
      else {
        construct(data_ + size_, *(data_ + size_ - 1));
        ++size_;
        copy_backward(p, data_ + size_ - 2, data_ + size_ - 1);
        *p = val;
      }
    }
    else {
      sz_t old_size = size_;
      sz_t new_capacity = capacity_ * 3 / 2;
      Ptr new_data = Alloc::allocate(new_capacity);
      try {
        uninitialized_copy(data_, p, new_data);
        construct(p, val);
        ++size_;
        uninitialized_copy(p, data_ + old_size, p + 1);
      }
      catch (...) {
        destroy(new_data, new_data + old_size + 1);
        Alloc::deallocate(new_data);
        throw;
      }

      destroy(begin(), end());
      Alloc::deallocate(data_);

      capacity_ = new_capacity;
      data_ = new_data;
    }
  }

  void insert_aux(ConstIter pos, ValueType&& val) {
    Ptr p = data_ + (pos - begin());
    if (size_ < capacity_) {
      if (p == data_ + size_) {
        construct(data_ + size_++, std::move(val));
      }
      else {
        construct(data_ + size_, *(data_ + size_ - 1));
        ++size_;
        copy_backward(p, data_ + size_ - 2, data_ + size_ - 1);
        *p = std::move(val);
      }
    }
    else {
      sz_t old_size = size_;
      sz_t new_capacity = capacity_ * 3 / 2;
      Ptr new_data = Alloc::allocate(new_capacity);
      try {
        uninitialized_copy(data_, p, new_data);
        construct(p, std::move(val));
        ++size_;
        uninitialized_copy(p, data_ + old_size, p + 1);
      }
      catch (...) {
        destroy(new_data, new_data + old_size + 1);
        Alloc::deallocate(new_data);
        throw;
      }

      destroy(begin(), end());
      Alloc::deallocate(data_);

      capacity_ = new_capacity;
      data_ = new_data;
    }
  }

  template <typename... Args> void insert_aux(ConstIter pos, Args&&... args) {
    Ptr p = data_ + (pos - begin());
    if (size_ < capacity_) {
      if (p == data_ + size_) {
        construct(data_ + size_++, std::forward<Args>(args)...);
      }
      else {
        construct(data_ + size_, *(data_ + size_ - 1));
        ++size_;
        copy_backward(p, data_ + size_ - 2, data_ + size_ - 1);
        construct(p, std::forward<Args>(args)...);
      }
    }
    else {
      sz_t old_size = size_;
      sz_t new_capacity = capacity_ * 3 / 2;
      Ptr new_data = Alloc::allocate(new_capacity);
      try {
        uninitialized_copy(data_, p, new_data);
        construct(p, std::forward<Args>(args)...);
        ++size_;
        uninitialized_copy(p, data_ + old_size, p + 1);
      }
      catch (...) {
        destroy(new_data, new_data + old_size + 1);
        Alloc::deallocate(new_data);
        throw;
      }

      destroy(begin(), end());
      Alloc::deallocate(data_);

      capacity_ = new_capacity;
      data_ = new_data;
    }
  }
public:
  ArrayList(sz_t capacity = kDefCapacity) noexcept
    : capacity_(capacity < kDefCapacity ? kDefCapacity : capacity) {
    data_ = Alloc::allocate(capacity_);
  }

  ~ArrayList() noexcept {
    destructor_aux();
  }

  ArrayList(sz_t count, const ValueType& value) noexcept
    : size_(count)
    , capacity_(count < kDefCapacity ? kDefCapacity : count) {
    data_ = Alloc::allocate(capacity_);
    uninitialized_fill(begin(), size_, value);
  }

  ArrayList(const ArrayList<Tp>& r) noexcept
    : size_(r.size_)
    , capacity_(r.capacity_) {
    data_ = Alloc::allocate(capacity_);
    uninitialized_copy(r.begin(), r.end(), begin());
  }

  ArrayList(ArrayList<Tp>&& r) noexcept {
    r.swap(*this);
  }

  inline ArrayList<Tp>& operator=(const ArrayList<Tp>& r) noexcept {
    if (this != &r) {
      destroy_aux();

      size_ = r.size_;
      capacity = r.capacity_;
      data_ = Alloc::allocate(capacity_);
      uninitialized_copy(r.begin(), r.end(), begin());
    }
    return *this;
  }

  inline ArrayList<Tp>& operator=(ArrayList<Tp>&& r) noexcept {
    if (this != &r) {
      destroy_aux();

      r.swap(*this);
    }
    return *this;
  }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline sz_t capacity() const noexcept { return capacity_; }
  inline Ptr data() noexcept { return data_; }
  inline ConstPtr data() const noexcept { return data_; }
  inline Ref at(sz_t i) noexcept { return data_[i]; }
  inline ConstRef at(sz_t i) const noexcept { return data_[i]; }
  inline Ref operator[](sz_t i) noexcept { return data_[i]; }
  inline ConstRef operator[](sz_t i) const noexcept { return data_[i]; }
  inline Iter begin() noexcept { return data_; }
  inline ConstIter begin() const noexcept { return data_; }
  inline Iter end() noexcept { return data_ + size_; }
  inline ConstIter end() const noexcept { return data_ + size_; }
  inline Ref get_head() noexcept { return *begin(); }
  inline ConstRef get_head() const noexcept { return *begin(); }
  inline Ref get_tail() noexcept { return *(end() - 1); }
  inline ConstRef get_tail() const noexcept { return *(end() - 1); }

  inline void clear() noexcept {
    destroy(begin(), end());
    size_ = 0;
  }

  void swap(ArrayList<Tp>& r) noexcept {
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

  void append(const ValueType& x) {
    if (size_ >= capacity_)
      regrow();
    construct(&data_[size_++], x);
  }

  void append(ValueType&& x) {
    if (size_ >= capacity_)
      regrow();
    construct(&data_[size_++], std::move(x));
  }

  template <typename... Args> void append(Args&&... args) {
    if (size_ >= capacity_)
      regrow();
    construct(&data_[size_++], std::forward<Args>(args)...);
  }

  ValueType pop() {
    ValueType r = data_[size_ - 1];
    destroy(&data_[--size_]);
    return r;
  }

  void insert(ConstIter pos, const ValueType& x) {
    insert_aux(pos, x);
  }

  void insert(ConstIter pos, ValueType&& x) {
    insert_aux(pos, std::move(x));
  }

  template <typename... Args> void insert(ConstIter pos, Args&&... args) {
    insert_aux(pos, std::forward<Args>(args)...);
  }

  void erase(ConstIter pos) {
    if (pos + 1 != end())
      copy(pos + 1, data_ + size_, pos);
    --size_;
    destroy(data_ + size_);
  }

  template <typename Visitor> inline void for_each(Visitor&& visitor) {
    for (sz_t i = 0; i < size_; ++i)
      visitor(data_[i]);
  }
};

}
