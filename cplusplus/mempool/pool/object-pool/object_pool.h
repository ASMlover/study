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
#ifndef __OBJECT_POOL_HEADER_H__
#define __OBJECT_POOL_HEADER_H__

template <typename T>
class object_pool_t {
  enum { GROW_SIZE = 128 };
  object_pool_t<T>* next_;

  object_pool_t(const object_pool_t&);
  object_pool_t& operator =(const object_pool_t&);
private:
  void 
  grow_free_space(void)
  {
    size_t size = (sizeof(T) > sizeof(object_pool_t<T>*) 
        ? sizeof(T) : sizeof(object_pool_t<T>*));

    object_pool_t<T>* node = (object_pool_t<T>*)malloc(size);
    assert(NULL != node);

    next_ = node;
    for (int i = 0; i < GROW_SIZE; ++i) {
      node->next_ = (object_pool_t<T>*)malloc(size);
      assert(NULL != node->next_);

      node = node->next_;
    }
    node->next_ = NULL;
  }
public:
  object_pool_t(void)
  {
    grow_free_space();
  }

  ~object_pool_t(void)
  {
    object_pool_t<T>* node;
    while (NULL != next_) {
      node = next_;
      next_ = next_->next_;
      free(node);
    }
  }

  inline void* 
  alloc(size_t size) 
  {
    if (NULL == next_)
      grow_free_space();

    object_pool_t<T>* head =  next_;
    next_ = next_->next_;

    return (void*)head;
  }

  inline void 
  dealloc(void* ptr)
  {
    assert(NULL != ptr);

    object_pool_t<T>* node = (object_pool_t<T>*)ptr;
    node->next_ = next_;
    next_ = node;
  }
};

#endif  //! __OBJECT_POOL_HEADER_H__
