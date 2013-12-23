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
#ifndef __EL_ALLOCATOR_HEADER_H__
#define __EL_ALLOCATOR_HEADER_H__

namespace el {

struct Memory;
class SpinLock;
class Allocator 
  : public Singleton<Allocator>, private NonCopyable {
  enum {
    ALIGN       = 8, 
    MAX_BYTES   = 1024, 
    NFREELISTS  = MAX_BYTES / ALIGN, 
    MAX_NUMBER  = 64, 
    PREFIX_SIZE = sizeof(size_t), 
  };

  Memory*   free_list_[NFREELISTS];
  void**    chunk_list_;
  size_t    chunk_count_;
  size_t    chunk_storage_;
  SpinLock* spinlock_;

  Allocator(const Allocator&);
  Allocator& operator =(const Allocator&);

  inline size_t 
  FreeListIndex(size_t bytes)
  {
    return ((bytes + (ALIGN - 1)) / ALIGN - 1);
  }

  Memory* AllocChunk(size_t index);
  void InsertChunk(void* chunk);
public:
  explicit Allocator(void);
  ~Allocator(void);

  void* Malloc(size_t bytes);
  void Free(void* ptr);
};

#define NEW(s)  Allocator::Instance().Malloc((s))
#define DEL(p)  Allocator::Instance().Free((p))



class SmallAllocator {
public:
  static void* operator new(size_t bytes)
  {
    return NEW(bytes);
  }

  static void operator delete(void* ptr, size_t bytes)
  {
    DEL(ptr);
  }
};

}

#endif  //! __EL_ALLOCATOR_HEADER_H__
