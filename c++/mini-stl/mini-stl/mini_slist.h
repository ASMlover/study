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
#ifndef __MINI_STL_SINGLE_LIST_HEADER_H__
#define __MINI_STL_SINGLE_LIST_HEADER_H__

#include <assert.h>
#include "mini_def.h"

namespace mini_stl {

template <typename T>
class SList {
  struct Node {
    Node* next;
    T     data;
  };

  Node*     front_;
  Node*     rear_;
  uint32_t  size_;
public:
  typedef T&        reference;
  typedef const T&  const_reference;

  class const_iterator {
  protected:
    Node* current_;

    const_iterator(Node* p) 
      : current_(p)
    {
    }

    friend class SList<T>;
  public:
    const_iterator(void)
      : current_(nullptr)
    {
    }

    const_reference operator *(void) const 
    {
      return current_->data;
    }

    const_iterator& operator ++(void)
    {
      current_ = current_->next;
      return *this;
    }

    const_iterator operator ++(int)
    {
      const_iterator old = *this;
      ++(*this);

      return old;
    }

    bool operator ==(const const_iterator& x) const 
    {
      return current_ == x.current_;
    }

    bool operator !=(const const_iterator& x) const
    {
      return !(x == *this);
    }
  };

  class iterator : public const_iterator {
  protected:
    iterator(Node* p)
      : current_(p)
    {
    }

    friend class SList<T>;
  public:
    iterator(void)
    {
    }

    reference operator *(void)
    {
      return current_->data;
    }

    const_reference operator *(void) const 
    {
      return current_->data;
    }

    iterator& operator ++(void)
    {
      current_ = current_->next;
      return *this;
    }

    iterator operator ++(int)
    {
      iterator old = *this;
      ++(*this);

      return old;
    }
  };
public:
  SList(void) 
    : front_(nullptr)
    , rear_(nullptr)
    , size_(0)
  {
  }

  ~SList(void)
  {
    clear();
  }

  SList(const SList& x)
    : front_(nullptr)
    , rear_(nullptr)
    , size_(0)
  {
    for (const_iterator it = x.begin(); it != x.end(); ++it)
      push_back(*it);
  }

  List& operator ==(const SList& x) 
  {
    if (this == &x)
      return *this;

    clear();
    for (const_iterator it = x.begin(); it != x.end(); ++it)
      push_back(*it);

    return *this;
  }
public:
  void clear(void)
  {
    Node* node;
    while (nullptr != front_) {
      node = front_;
      front_ = front_->next;
      delete node;
    }

    front_ = nullptr;
    rear_ = nullptr;
    size_ = 0;
  }

  bool empty(void) const 
  {
    return nullptr == front_;
  }

  uint32_t size(void) const 
  {
    return size_;
  }

  iterator begin(void) 
  {
    return iterator(front_);
  }

  const_iterator begin(void) const 
  {
    return const_iterator(front_);
  }

  iterator end(void) 
  {
    return iterator();
  }

  const_iterator end(void) const 
  {
    return const_iterator();
  }

  reference front(void)
  {
    return *begin();
  }

  const_reference front(void) const
  {
    return *begin();
  }

  reference back(void) 
  {
    return *(iterator(rear_));
  }

  const_reference back(void) const 
  {
    return *(const_iterator(rear_));
  }

  void push_back(const T& x)
  {
    Node* node = new Node;
    assert(nullptr != node);

    node->next = nullptr;
    node->data = x;
    if (nullptr == front_)
      front_ = rear_ = node;
    else {
      rear_->next = node;
      rear_ = node;
    }
    ++size_;
  }

  void push_front(const T& x)
  {
    Node* node = new Node;
    assert(nullptr != node);

    node->next = front_;
    node->data = x;
    front_ = node;
    if (nullptr == rear_)
      rear_ = node;
    ++size_;
  }

  void pop_back(void)
  {
    assert(nullptr != front_);

    Node* node = front_;
    front_ = front_->next;
    delete node;

    --size_;
  }
};

}

#endif  //! __MINI_STL_SINGLE_LIST_HEADER_H__
