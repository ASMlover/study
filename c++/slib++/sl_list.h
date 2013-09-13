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
#ifndef __SL_LIST_HEADER_H__
#define __SL_LIST_HEADER_H__

#include <assert.h>
#include "sl_allocator.h"

namespace sl {

template <typename T>
class list_t {
  struct node_t {
    node_t* prev;
    node_t* next;
    T value;
  };

  node_t head_;
  size_t size_;
public:
  typedef T&        reference_t;
  typedef const T&  const_reference_t;

  class const_iterator_t {
  protected:
    node_t* current_;

    const_iterator_t(node_t* node)
      : current_(node)
    {
    }

    friend class list_t<T>;
  public:
    const_iterator_t(void)
      : current_(NULL)
    {
    }

    const_reference_t 
    operator *(void) const 
    {
      return current_->value;
    }

    const_iterator_t& 
    operator ++(void)
    {
      current_ = current_->next;
      return *this;
    }

    const_iterator_t 
    operator ++(int) 
    {
      const_iterator_t old = *this;
      ++(*this);

      return old;
    }

    const_iterator_t& 
    operator --(void)
    {
      current_ = current_->prev;
      return *this;
    }

    const_iterator_t 
    operator --(int)
    {
      const_iterator_t old = *this;
      --(*this);

      return old;
    }

    bool 
    operator ==(const const_iterator_t& x) const 
    {
      return (current_ == x.current_);
    }

    bool 
    operator !=(const const_iterator_t& x) const 
    {
      return !(x == *this);
    }
  };

  class iterator_t : public const_iterator_t {
  protected:
    iterator_t(node_t* node)
      : const_iterator_t(node)
    {
    }

    friend class list_t<T>;
  public:
    iterator_t(void)
    {
    }

    reference_t 
    operator *(void)
    {
      return this->current_->value;
    }

    const_reference_t 
    operator *(void) const
    {
      return this->current_->value;
    }

    iterator_t& 
    operator ++(void)
    {
      this->current_ = this->current_->next;
      return *this;
    }

    iterator_t 
    operator ++(int)
    {
      iterator_t old = *this;
      ++(*this);

      return old;
    }

    iterator_t& 
    operator --(void)
    {
      this->current_ = this->current_->prev;
      return *this;
    }

    iterator_t 
    operator --(int) 
    {
      iterator_t old = *this;
      --(*this);

      return old;
    }
  };
public:
  list_t(void)
    : size_(0)
  {
    head_.prev = &head_;
    head_.next = &head_;
  }

  ~list_t(void)
  {
    clear();
  }

  list_t(const list_t& x)
    : size_(x.size_)
  {
    head_.prev = &head_;
    head_.next = &head_;

    for (const_iterator_t it = x.begin(); it != x.end(); ++it)
      push_back(*it);
  }

  list_t& 
  operator =(const list_t& x) 
  {
    if (&x == this)
      return *this;

    clear();
    for (const_iterator_t it = x.begin(); it != x.end(); ++it)
      push_back(*it);
  }
public:
  void 
  clear(void)
  {
    erase(begin(), end());
  }

  bool 
  empty(void) const 
  {
    return (0 == size_);
  }

  size_t 
  size(void) const 
  {
    return size_;
  }

  iterator_t 
  begin(void)
  {
    return iterator_t(head_.next);
  }

  const_iterator_t 
  begin(void) const 
  {
    return const_iterator_t(head_.next);
  }

  iterator_t 
  end(void)
  {
    return iterator_t(&head_);
  }

  const_iterator_t 
  end(void) const
  {
    return const_iterator_t(&head_);
  }

  reference_t 
  front(void) 
  {
    return *begin();
  }

  const_reference_t 
  front(void) const 
  {
    return *begin();
  }

  reference_t 
  back(void)
  {
    return *(--end());
  }

  const_reference_t 
  back(void) const 
  {
    return *(--end());
  }

  void 
  push_back(const T& value)
  {
    insert(end(), value);
  }

  void 
  push_front(const T& value)
  {
    insert(begin(), value);
  }

  void 
  pop_back(void)
  {
    iterator_t pos = end();
    erase(--pos);
  }

  void 
  pop_front(void)
  {
    erase(begin());
  }
public:
  void 
  insert(iterator_t pos, const T& value)
  {
    node_t* node = (node_t*)sl_new(sizeof(node_t));
    assert(NULL != node);

    node->value = value;
    node->prev = pos.current_->prev;
    node->next = pos.current_;
    pos.current_->prev->next = node;
    pos.current_->prev = node;

    ++size_;
  }

  iterator_t 
  erase(iterator_t pos)
  {
    node_t* prev = pos.current_->prev;
    node_t* next = pos.current_->next;
    prev->next = next;
    next->prev = prev;
    sl_del(pos.current_);

    --size_;
    return iterator_t(next);
  }

  void 
  erase(iterator_t from, iterator_t to)
  {
    for (iterator_t it = from; it != to; )
      it = erase(it);
  }
};

}

#endif  //! __SL_LIST_HEADER_H__
