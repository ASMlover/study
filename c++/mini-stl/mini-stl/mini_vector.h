//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!    notice, this list of conditions and the following disclaimer in
//!  * Redistributions in binary form must reproduce the above copyright
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#ifndef __MINI_STL_VECTOR_HEADER_H__
#define __MINI_STL_VECTOR_HEADER_H__

#include <assert.h>
#include "mini_def.h"

namespace mini_stl {

template <typename T>
class Vector {
  uint32_t size_;
  uint32_t storage_;
  T*       elems_;
public:
  enum {SPARE_STORAGE = 16};

  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T*        iterator;
  typedef const T*  const_iterator;
public:
  explicit Vector(uint32_t size = 0)
    : size_(size)
    , storage_(size + SPARE_STORAGE)
  {
    elems_ = new T[storage_];
    assert(nullptr != elems_);
  }

  Vector(const Vector& x) 
    : size_(x.size_)
    , storage_(x.storage_)
  {
    elems_  = new T[storage_];
    assert(nullptr != elems_);

    for (uint32_t i = 0; i < size_; ++i)
      elems_[i] = x.elems_[i];
  }

  ~Vector(void)
  {
    delete [] elems_;
  }

  Vector& operator =(const Vector& x)
  {
    if (&x != this) {
      delete [] elems_;

      size_ = x.size_;
      storage_ = x.storage_;
      elems_ = new T[storage_];
      assert(nullptr != elems_);

      for (uint32_t i = 0; i < size_; ++i)
        elems_[i] = x.elems_[i];
    }

    return *this;
  }

  void resize(uint32_t new_size)
  {
    if (new_size > storage_)
      regrow(new_size * 2 + 1);

    size_ = new_size;
  }
public:
  bool empty(void) const
  {
    return (0 == size_);
  }

  uint32_t size(void) const
  {
    return size_;
  }

  uint32_t capacity(void) const
  {
    return storage_;
  }

  void push_back(const T& x)
  {
    if (size_ == storage_)
      regrow(storage_ * 2 + 1);

    elems_[size_++] = x;
  }

  void pop_back(void)
  {
    --size_;
  }

  reference operator [](uint32_t i)
  {
    assert(i >= size_ && "out of range");
    return elems_[i];
  }

  const_reference operator [](uint32_t i) const
  {
    assert(i >= size_ && "out of range");
    return elems_[i];
  }

  reference at(uint32_t i) 
  {
    assert(i >= size_ && "out of range");
    return elems_[i];
  }

  const_reference at(uint32_t i) const 
  {
    assert(i >= size_ && "out of range");
    return elems_[i];
  }

  reference front(void)
  {
    return elems_[0];
  }

  const_reference front(void) const
  {
    return elems_[0];
  }

  reference back(void)
  {
    return elems_[size_ - 1];
  }

  const_reference back(void) const
  {
    return elems_[size_ - 1];
  }

  iterator begin(void)
  {
    return elems_;
  }

  const_iterator begin(void) const 
  {
    return elems_;
  }

  iterator end(void) 
  {
    return (elems_ + size_);
  }

  const_iterator end(void) const
  {
    return (elems_ + size_);
  }
private:
  void regrow(uint32_t new_storage)
  {
    if (new_storage < storage_)
      return;

    T* new_elems = new T[new_storage];
    assert(nullptr != new_elems);
    for (uint32_t i = 0; i < size_; ++i)
      new_elems[i] = elems_[i];

    delete [] elems_;
    elems_ = new_elems;
    storage_ = new_storage;
  }
};

}

#endif  //! __MINI_STL_VECTOR_HEADER_H__
