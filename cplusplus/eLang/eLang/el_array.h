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
#ifndef __EL_ARRAY_HEADER_H__
#define __EL_ARRAY_HEADER_H__

namespace el {

template <typename _Tp> class Array {
  static const int MIN_CAPACITY = 16;
  static const int GROW_FACTOR  = 2;

  _Tp*  items_;
  int   count_;
  int   capacity_;
public:
  Array(void)
    : items_(nullptr)
    , count_(0)
    , capacity_(0) {
  }

  explicit Array(int capacity)
    : items_(nullptr)
    , count_(0)
    , capacity_(0) {
    EnsureCapacity(capacity);
  }

  Array(int count, const _Tp& fill_with)
    : items_(nullptr)
    , count_(0)
    , capacity_(0) {
    EnsureCapacity(count);

    for (auto i = 0; i < count; ++i)
      items_[i] = fill_with;
    count_ = count;
  }

  Array(const Array<_Tp>& other)
    : items_(nullptr)
    , count_(0)
    , capacity_(0) {
    Append(other);
  }

  ~Array(void) {
    Clear();
  }

  void Clear(void) {
    if (nullptr != items_) {
      delete [] items_;
      items_ = nullptr;
    }
    count_ = 0;
    capacity_ = 0;
  }

  inline int Count(void) const {
    return count_;
  }

  inline int Capacity(void) const {
    return capacity_;
  }

  inline bool IsEmpty(void) const {
    return (0 == count_);
  }

  inline int IndexOf(const _Tp& value) const {
    for (auto i = 0; i < count_; ++i) {
      if (items_[i] == value)
        return i;
    }
    return -1;
  }

  void Append(const _Tp& value) {
    EnsureCapacity(count_ + 1);
    items_[count_++] = value;
  }

  void Append(const Array<_Tp>& other) {
    EnsureCapacity(count_ + other.count_);

    for (auto i = 0; i < other.count_; ++i)
      items_[count_++] = other[i];
  }

  void RemoveAt(int index) {
    if (index < 0)
      index = count_ + index;
    EL_ASSERT_RANGE(index, count_);

    for (auto i = index; i < count_ - 1; ++i)
      items_[i] = items_[i + 1];

    items_[count_ - 1] = _Tp();
    --count_;
  }

  void Truncate(int count) {
    EL_ASSERT(count >= 0, "Cannot truncate to a negative count.");

    if (count >= count_)
      return;

    for (auto i = count; i < count_; ++i)
      items_[i] = _Tp();

    count_ = count;
  }

  void Reverse(void) {
    for (auto i = 0; i < count_ / 2; ++i)
      std::swap(items_[i], items_[count_ - 1 -i]);
  }

  inline Array<_Tp>& operator=(const Array<_Tp>& other) {
    if (&other != this) {
      Clear();
      Append(other);
    }

    return *this;
  }

  inline _Tp& operator[](int index) {
    if (index < 0)
      index = count_ + index;
    EL_ASSERT_RANGE(index, count_);

    return items_[index];
  }

  inline const _Tp& operator[](int index) const {
    if (index < 0)
      index = count_ + index;
    EL_ASSERT_RANGE(index, count_);

    return items_[index];
  }
private:
  void EnsureCapacity(int desired_capacity) {
    if (capacity_ >= desired_capacity)
      return;

    int capacity = capacity_ < MIN_CAPACITY ? MIN_CAPACITY : capacity_;
    while (capacity < desired_capacity)
      capacity *= GROW_FACTOR;

    _Tp* new_items = new _Tp[capacity];
    if (nullptr != items_) {
      for (auto i = 0; i < count_; ++i)
        new_items[i] = items_[i];

      delete [] items_;
    }

    items_ = new_items;
    capacity_ = capacity;
  }
};

}

#endif  // __EL_ARRAY_HEADER_H__
