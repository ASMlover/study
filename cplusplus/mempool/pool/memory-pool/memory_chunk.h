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
#ifndef __MEMORY_CHUNK_HEADER_H__
#define __MEMORY_CHUNK_HEADER_H__ 


typedef unsigned char byte_t;
class memory_chunk_t {
  enum { CHUNK_SIZE_DEF = 4096 };
  memory_chunk_t* next_;

  size_t  allocated_;
  size_t  chunk_size_;
  byte_t* memory_;
private:
  memory_chunk_t(const memory_chunk_t&);
  memory_chunk_t& operator =(const memory_chunk_t&);
public:
  memory_chunk_t(memory_chunk_t* chunk, size_t chunk_size = CHUNK_SIZE_DEF);
  ~memory_chunk_t(void);

  memory_chunk_t* next(void);
  size_t free_space(void) const;

  void* alloc(size_t size);
  void dealloc(void* ptr);
};


#endif  //! __MEMORY_CHUNK_HEADER_H__
