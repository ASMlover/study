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
#ifndef __SL_ALLOCATOR_HEADER_H__
#define __SL_ALLOCATOR_HEADER_H__

#include "sl_noncopyable.h"

namespace sl {

struct memory_t;
class spinlock_t;
class allocator_t : noncopyable {
  enum {
    ALIGN       = 8, 
    MAX_BYTES   = 1024, 
    NFREELISTS  = MAX_BYTES / ALIGN, 
    MAX_NUMBER  = 64, 
    PREFIX_SIZE = sizeof(size_t), 
  };

  memory_t*   free_list_[NFREELISTS];
  void**      chunk_list_;
  size_t      chunk_count_;
  size_t      chunk_storage_;
  spinlock_t* spinlock_;

  inline size_t 
  freelist_index(size_t bytes)
  {
    return ((bytes + (ALIGN - 1)) / ALIGN - 1);
  }

  memory_t* alloc_chunk(size_t index);
  void insert_chunk(void* chunk);
public:
  allocator_t(void);
  ~allocator_t(void);

  static allocator_t& singleton(void);

  void* alloc(size_t bytes);
  void dealloc(void* ptr);
};

#define malloc(s)   allocator_t::singleton().alloc((s))
#define free(p)     allocator_t::singleton().dealloc((p))

}

#endif  //! __SL_ALLOCATOR_HEADER_H__
