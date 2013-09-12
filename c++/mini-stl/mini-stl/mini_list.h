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
#ifndef __MINI_STL_LIST_HEADER_H__
#define __MINI_STL_LIST_HEADER_H__

#include <assert.h>
#include "mini_def.h"

namespace mini_stl {

template <typename T>
class List {
  struct Node {
    Node* prev;
    Node* next;
    T     data;
  };

  Node      head_;
  uint32_t  size_;
public:
  typedef T&        reference;
  typedef const T&  const_reference;

  class const_iterator {
  protected:
    Node* current_;

    const_iterator(Node* p) 
      : current_(nullptr)
    {
    }

    friend class List<T>;
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

    const_iterator& operator --(void)
    {
      current_ = current_->prev;
      return *this;
    }

    const_iterator operator --(int)
    {
      const_iterator old = *this;
      --(*this);

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

    friend class List<T>;
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

    iterator& operator --(void) 
    {
      current_ = current_->prev;
      return *this;
    }

    iterator operator --(int)
    {
      iterator old = *this;
      --(*this);

      return old;
    }
  };
public:
  List(void)
    : size_(0)
  {
    head_->prev = &head_;
    head_->next = &head_;
  }

  ~List(void)
  {
    clear();
  }

  List(const List& x)
    : size_(0)
  {
    for (const_iterator it = x.begin(); it != x.end(); ++it)
      push_back(*it);
  }

  List& operator =(const List& x) 
  {
    if (&x == this)
      return *this;

    clear();
    for (const_iterator it = x.begin(); it != x.end(); ++it)
      push_back(*it);

    return *this;
  }
public:
  void clear(void)
  {
    erase(begin(), end());
  }

  bool empty(void) const 
  {
    return (0 == size_);
  }

  uint32_t size(void) const 
  {
    return size_;
  }

  iterator begin(void)
  {
    return iterator(head_.next);
  }

  const_iterator begin(void) const 
  {
    return const_iterator(head_.next);
  }

  iterator end(void) 
  {
    return iterator(&head_);
  }

  const_iterator end(void) const 
  {
    return const_iterator(&head_);
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
    return *(--end());
  }

  const_reference back(void) const 
  {
    return *(--end());
  }

  void push_back(const T& x)
  {
    insert(end(), x);
  }

  void push_front(const T& x)
  {
    insert(begin(), x);
  }

  void pop_back(void)
  {
    iterator it = end();
    erase(--it);
  }

  void pop_front(void) 
  {
    erase(begin());
  }
public:
  void insert(iterator pos, const T& x)
  {
    Node* node = new Node;
    assert(nullptr != node);

    node->data = x;
    node->prev = pos.current_->prev;
    node->next = pos.current_;
    pos.current_->prev->next = node;
    pos.current_->prev = node;

    ++size_;
  }

  iterator erase(iterator pos)
  {
    Node* prev;
    Node* next;

    prev = pos.current_->prev;
    next = pos.current_->next;
    prev->next = next;
    next->prev = prev;
    delete pos.current_;

    --size_;
    return iterator(next);
  }

  void erase(iterator from, iterator to)
  {
    for (iterator it = from; it != to; )
      it = erase(it);
  }
};

}

#endif  //! __MINI_STL_LIST_HEADER_H__
