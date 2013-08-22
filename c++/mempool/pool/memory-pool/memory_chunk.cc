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
#include <assert.h>
#include <stdlib.h>
#include "memory_chunk.h"



memory_chunk_t::memory_chunk_t(memory_chunk_t* chunk, size_t chunk_size)
{
  next_ = chunk;
  allocated_ = 0;
  chunk_size_ = (chunk_size > CHUNK_SIZE_DEF ? chunk_size : CHUNK_SIZE_DEF);
  memory_ = (byte_t*)malloc(chunk_size_);
  assert(NULL != memory_);
}

memory_chunk_t::~memory_chunk_t(void)
{
  free(memory_);
}

memory_chunk_t* 
memory_chunk_t::next(void)
{
  return next_;
}

size_t 
memory_chunk_t::free_space(void) const 
{
  return (chunk_size_ - allocated_);
}

void* 
memory_chunk_t::alloc(size_t size)
{
  void* addr = memory_ + allocated_;
  allocated_ += size;

  return addr;
}

void 
memory_chunk_t::dealloc(void* ptr)
{
  assert(NULL != ptr);
}
