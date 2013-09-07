//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
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
#ifndef __SL_ARRAY_HEADER_H__
#define __SL_ARRAY_HEADER_H__ 

#include <assert.h>
#include "sl_allocator.h"

namespace sl {

template <typename T>
class array_t {
  enum { DEF_SPACE_STIRAGE = 16 };
  size_t size_;
  size_t storage_;
  T* elems_;
public:
  typedef T&        reference_t;
  typedef const T&  const_reference_t;
  typedef T*        iterator_t;
  typedef const T*  const_iterator_t;
public:
  explicit array_t(void)
    : size_(0)
    , storage_(DEF_SPACE_STIRAGE)
  {
    elems_ = (T*)sl_new(sizeof(T) * storage_);
    assert(NULL != elems_);
  }

  array_t(const array_t& x)
    : size_(x.size_)
    , storage_(x.storage_)
  {
    elems_ = (T*)sl_new(sizeof(T) * storage_);
    assert(NULL != elems_);

    for (size_t i = 0; i < size_; ++i)
      elems_[i] = x.elems_[i];
  }

  ~array_t(void)
  {
    sl_del(elems_);
  }

  array_t& operator =(const array_t& x)
  {
    if (&x != this) {
      sl_del(elems_);

      size_ = x.size_;
      storage_ = x.storage_;
      elems_ = (T*)sl_new(sizeof(T) * storage_);
      assert(NULL != elems_);

      for (size_t i = 0; i < size_; ++i)
        elems_[i] = x.elems_[i];
    }
  }
public:
  bool empty(void) const 
  {
    return (0 == size_);
  }

  size_t size(void) const 
  {
    return size_;
  }

  size_t capacity(void) const 
  {
    return storage_;
  }

  void push_back(const T& x) 
  {
    if (size_ == storage_)
      regrow(storage_ * 2);

    elems_[size_++] = x;
  }

  void pop_back(void)
  {
    --size_;
  }

  reference_t operator [](size_t i) 
  {
    return elems_[i];
  }

  const_reference_t operator [](size_t i) const 
  {
    return elems_[i];
  }

  reference_t at(size_t i)
  {
    return elems_[i];
  }

  const_reference_t at(size_t i) const 
  {
    return elems_[i];
  }

  reference_t front(void)
  {
    return elems_[0];
  }

  const_reference_t front(void) const 
  {
    return elems_[0];
  }

  reference_t back(void) 
  {
    return elems_[size_ - 1];
  }

  const_reference_t back(void) const 
  {
    return elems_[size_ - 1];
  }

  iterator_t begin(void) 
  {
    return elems_;
  }

  const_iterator_t begin(void) const 
  {
    return elems_;
  }

  iterator_t end(void) 
  {
    return (elems_ + size_);
  }

  const_iterator_t end(void) const 
  {
    return (elems_ + size_);
  }
private:
  void regrow(size_t new_storage)
  {
    if (new_storage < storage_)
      return;

    T* new_elems = (T*)sl_new(sizeof(T) * new_storage);
    assert(NULL != new_elems);

    for (size_t i = 0; i < size_; ++i)
      new_elems[i] = elems_[i];

    sl_del(elems_);
    elems_ = new_elems;
    storage_ = new_storage;
  }
};

}

#endif  //! __SL_ARRAY_HEADER_H__
