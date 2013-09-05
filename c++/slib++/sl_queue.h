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
#ifndef __SL_QUEUE_HEADER_H__
#define __SL_QUEUE_HEADER_H__ 

#include "sl_allocator.h"

namespace sl {

template <typename T>
class queue_t : noncopyable {
  struct node_t {
    node_t* next;
    T value;
  };

  node_t* head_;
  node_t* tail_;
  size_t  size_;
public:
  queue_t(void)
    : head_(NULL)
    , tail_(NULL)
    , size_(0)
  {
  }

  ~queue_t(void)
  {
  }

  bool 
  empty(void) const 
  {
    return (NULL == head_);
  }

  void 
  push(const T& value) 
  {
    node_t* node = (node_t*)sl_new(sizeof(node_t));
    node->next = NULL;
    node->value = value;

    if (NULL == head_)
      head_ = tail_ = node;
    else {
      tail_->next = node;
      tail_ = node;
    }

    ++size_;
  }

  T& 
  pop(void)
  {
    assert(NULL != head_);

    node_t* node = head_;
    head_ = head_->next;
    T value = node->value;
    sl_del(node);
    --size_;

    return value;
  }
};
}

#endif  //! __SL_QUEUE_HEADER_H__
