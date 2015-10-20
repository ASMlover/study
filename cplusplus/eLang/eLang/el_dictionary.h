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
#ifndef __EL_DICTIONARY_HEADER_H__
#define __EL_DICTIONARY_HEADER_H__

namespace el {

template <typename _Key, typename _Value>
class Dictionary : private UnCopyable {
  static const int MAX_LOAD_PERCENT = 75;
  static const int MIN_CAPACITY     = 16;
  static const int GROW_FACTOR      = 2;

  struct Pair {
    _Key    key;
    _Value  value;
  };

  Pair* table_;
  int   count_;
  int   capacity_;
public:
  Dictionary(void)
    : table_(nullptr)
    , count_(0)
    , capacity_(0) {
  }

  ~Dictionary(void) {
    if (nullptr != table_)
      delete [] table_;
  }

  inline void Clear(void) {
    if (nullptr != table_) {
      delete [] table_;
      table_ = nullptr;
    }
    count_ = 0;
    capacity_ = 0;
  }

  inline bool IsEmpty(void) const {
    return (0 == count_);
  }

  inline int Count(void) const {
    return count_;
  }

  inline int Capacity(void) const {
    return capacity_;
  }

  void Insert(const _Key& key, const _Value& value) {
    ++count_;
    EnsureCapacity();

    int index = static_cast<int>(key.HashCode() % 0x7fffffff) % capacity_;
    while ((table_[index].key.Length() > 0) && (table_[index].key != key))
      index = (index + 1) % capacity_;

    table_[index].key = key;
    table_[index].value = value;
  }

  bool Remove(const _Key& key) {
    int index = IndexOf(key);
    if (-1 == index)
      return false;

    table_[index].key = _Key();
    table_[index].value = _Value();

    --count_;
    return true;
  }

  bool Find(const _Key& key, _Value& value) {
    int index = IndexOf(key);
    if (-1 == index)
      return false;

     value = table_[index].value;
     return true;
  }

  bool Replace(const _Key& key, const _Value& value) {
    int index = IndexOf(key);
    if (-1 == index)
      return false;

    table_[index].value = value;
    return true;
  }
private:
  int IndexOf(const _Key& key) {
    if (0 == capacity_)
      return -1;
    int index = static_cast<int>(key.HashCode() & 0x7fffffff) % capacity_;

    while (true) {
      if (table_[index].key == key)
        break;

      if (table_[index].key.IsEmpty())
        return -1;

      index = (index + 1) % capacity_;
    }

    return index;
  }

  void EnsureCapacity(void) {
    if (count_ <= capacity_ * MAX_LOAD_PERCENT / 100)
      return;

    int old_capacity = capacity_;
    capacity_ = capacity_ >= MIN_CAPACITY
      ? capacity_ * GROW_FACTOR : MIN_CAPACITY;

    Pair* new_table = new Pair[capacity_];
    if (nullptr != table_) {
      for (auto i = 0; i < old_capacity; ++i) {
        if (table_[i].key.Length() > 0)
          Insert(table_[i].key, table_[i].value);
      }

      delete [] table_;
    }

    table_ = new_table;
  }
};

template <typename _Value> class IdDictionary : private UnCopyable {
  static const int MAX_LOAD_PERCENT = 75;
  static const int MIN_CAPACITY     = 16;
  static const int GROW_FACTOR      = 2;

  struct Pair {
    StringId  key;
    _Value    value;

    Pair(void)
      : key(EL_NOSTRING) {
    }
  };
  Pair* table_;
  int   count_;
  int   capacity_;
public:
  IdDictionary(void)
    : table_(nullptr)
    , count_(0)
    , capacity_(0) {
  }

  ~IdDictionary(void) {
    if (nullptr != table_)
      delete [] table_;
  }

  inline void Clear(void) {
    if (nullptr != table_) {
      delete [] table_;
      table_ = nullptr;
    }
    count_ = 0;
    capacity_ = 0;
  }

  inline bool IsEmpty(void) const {
    return (0 == count_);
  }

  inline int Count(void) const {
    return count_;
  }

  inline int Capacity(void) const {
    return capacity_;
  }

  void Insert(StringId key, const _Value& value) {
    ++count_;
    EnsureCapacity();

    int index = static_cast<int>(key & 0x7fffffff) % capacity_;
    while ((table_[index].key != EL_NOSTRING) && (table_[index].key != key))
      index = (index + 1) % capacity_;

    table_[index].key = key;
    table_[index].value = value;
  }

  bool Remove(StringId key) {
    int index = IndexOf(key);
    if (-1 == index)
      return false;

    table_[index].key = EL_NOSTRING;
    table_[index].value = _Value();

    --count_;
    return true;
  }

  bool Find(StringId key, _Value& value) {
    int index = IndexOf(key);
    if (-1 == index)
      return false;

    value = table_[index].value;
    return true;
  }

  StringId FindKeyForValue(const _Value& value) {
    for (auto i = 0; i < capacity_; ++i) {
      if (table_[i].value == value)
        return table_[i].key;
    }

    return EL_NOSTRING;
  }

  bool Replace(StringId key, const _Value& value) {
    int index = IndexOf(key);
    if (-1 == index)
      return false;

    table_[index].value = value;
    return true;
  }
private:
  int IndexOf(StringId key) {
    if (0 == capacity_)
      return -1;
    int index = static_cast<int>(key & 0x7fffffff) % capacity_;
    
    while (true) {
      if (table_[index].key == key)
        break;

      if (table_[index].key == EL_NOSTRING)
        return -1;

      index = (index + 1) % capacity_;
    }

    return index;
  }

  void EnsureCapacity(void) {
    if (count_ <= capacity_ * MAX_LOAD_PERCENT / 100)
      return;

    int old_capacity = capacity_;
    capacity_ = capacity_ >= MIN_CAPACITY
      ? capacity_ * GROW_FACTOR : MIN_CAPACITY;

    Pair* new_table = new Pair[capacity_];
    if (nullptr != table_) {
      for (auto i = 0; i < old_capacity; ++i) {
        if (table_[i].key != EL_NOSTRING)
          Insert(table_[i].key, table_[i].value);
      }

      delete [] table_;
    }

    table_ = new_table;
  }
};

}

#endif  // __EL_DICTIONARY_HEADER_H_
