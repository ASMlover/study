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
#ifndef __MINI_STL_QUEUE_HEADER_H__
#define __MINI_STL_QUEUE_HEADER_H__

#include <assert.h>
#include "mini_def.h"

namespace mini_stl {

template <typename T>
class Queue {
  struct Node {
    Node* next;
    T     data;
  };

  Node* head_;
  Node* tail_;

private:
  void clear(void)
  {
    Node* node;
    while (nullptr != head_) {
      node = head_;
      head_ = head_->next;

      delete node;
    }
  }
public:
  Queue(void) 
    : head_(nullptr)
    , tail_(nullptr)
  {
  }

  ~Queue(void)
  {
    clear();
  }
public:
  bool empty(void) const 
  {
    return (nullptr == head_);
  }

  void push(const T& x)
  {
    Node* node = new Node;
    assert(nullptr != node);

    node->next = nullptr;
    node->data = x;
    if (nullptr == head_) 
      head_ = tail_ = node;
    else {
      tail_->next = node;
      tail_ = node;
    }
  }

  void pop(void)
  {
    assert(nullptr != head_);

    Node* node = head_;
    head_ = head_->next;

    delete node;
  }

  T& top(void)
  {
    assert(nullptr != head_);

    return head_->data;
  }
};

}

#endif  //! __MINI_STL_QUEUE_HEADER_H__
