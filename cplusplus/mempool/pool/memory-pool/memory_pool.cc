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
#include <stdio.h>
#include "memory_chunk.h"
#include "memory_pool.h"



memory_pool_t::memory_pool_t(void)
{
  chunk_list_ = new memory_chunk_t(NULL);
  assert(NULL != chunk_list_);
}

memory_pool_t::~memory_pool_t(void)
{
  memory_chunk_t* node;
  while (NULL != chunk_list_) {
    node = chunk_list_;
    chunk_list_ = chunk_list_->next();
    delete node;
  }
}

void 
memory_pool_t::grow_free_space(size_t size)
{
  chunk_list_ = new memory_chunk_t(chunk_list_, size);
  assert(NULL != chunk_list_);
}

void* 
memory_pool_t::alloc(size_t size)
{
  if (size > chunk_list_->free_space())
    grow_free_space(size);

  return chunk_list_->alloc(size);
}

void 
memory_pool_t::dealloc(void* ptr)
{
  chunk_list_->dealloc(ptr);
}
