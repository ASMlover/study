/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "allocator.h"

#if defined(_MSC_VER)
  #define inline __inline
#endif


enum allocator_size_type {
  ALIGN       = 8, 
  MAX_BYTES   = 256,
  NFREELISTS  = MAX_BYTES / ALIGN, 
  MAX_NUMBER  = 64, 
};


struct memory_t {
  struct memory_t* next;
};
struct chunk_t {
  struct chunk_t*   next;
  struct memory_t*  data;
};


struct allocator_t {
  struct memory_t*  free_list[NFREELISTS];
  struct chunk_t*   chunk_list;
}; 



static inline size_t 
free_index(size_t bytes)
{
  return ((bytes + (ALIGN - 1)) / ALIGN - 1);
}

static inline size_t 
_min(size_t a, size_t b)
{
  return (a < b ? a : b);
}




static struct memory_t* 
alloc_chunk(size_t index)
{
  return NULL;
}

void* 
al_malloc(size_t bytes)
{
  return NULL;
}

void 
al_free(void* ptr)
{
}
