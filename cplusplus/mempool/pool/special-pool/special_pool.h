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

#ifndef __SPECIAL_POOL_HEADER_H__
#define __SPECIAL_POOL_HEADER_H__ 


struct free_space_t {
  free_space_t* next;
};


template <typename T> 
class special_pool_t {
  enum { GROW_SIZE = 32 };
  static free_space_t* free_space_;

  special_pool_t(const special_pool_t&);
  special_pool_t& operator =(const special_pool_t&);
private:
  static void 
  grow_free_space(void)
  {
    size_t size = (sizeof(T) > sizeof(free_space_t*)
        ? sizeof(T) : sizeof(free_space_t*));

    free_space_t* node = static_cast<free_space_t*>(malloc(size));
    free_space_ = node;
    for (int i = 0; i < GROW_SIZE; ++i) {
      node->next = static_cast<free_space_t*>(malloc(size));
      node = node->next;
    }
    node->next = 0;
  }
public:
  special_pool_t(void)
  {}
  ~special_pool_t(void)
  {}

  inline void* 
  operator new(size_t size)
  {
    if (0 == free_space_)
      grow_free_space();

    free_space_t* node = free_space_;
    free_space_ = free_space_->next;

    return static_cast<void*>(node);
  }

  inline void 
  operator delete(void* ptr, size_t size)
  {
    free_space_t* node = static_cast<free_space_t*>(ptr);
    node->next = free_space_;
    free_space_ = node;
  }
public:
  static void 
  init(void)
  {
    grow_free_space();
  }

  static void 
  destroy(void) 
  {
    free_space_t* node;
    while (0 != free_space_) {
      node = free_space_;
      free_space_ = free_space_->next;
      free(node);
    }
  }
};


#endif  //! __SPECIAL_POOL_HEADER_H__
