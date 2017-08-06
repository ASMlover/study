/*
 * Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pymem.h"

#define ALIGNMENT       8
#define ALIGNMENT_SHIFT 3
#define ALIGNMENT_MASK  (ALIGNMENT - 1)

#define INDEX2SIZE(I)   (((uint32_t)(I) + 1) << ALIGNMENT_SHIFT)
#define SIZE2INDEX(S)   (((uint32_t)(S) - 1) >> ALIGNMENT_SHIFT)

#define SMALL_REQUEST_THRESHOLD 512
#define NB_SMALL_SIZE_CLASSES   (SMALL_REQUEST_THRESHOLD / ALIGNMENT)

#define SYSTEM_PAGE_SIZE        (1024 * 4)
#define SYSTEM_PAGE_SIZE_MASK   (SYSTEM_PAGE_SIZE - 1)
#define POOL_SIZE               SYSTEM_PAGE_SIZE
#define POOL_SIZE_MASK          SYSTEM_PAGE_SIZE_MASK

#define ROUNDUP(x)              (((x) + ALIGNMENT_MASK) & ~ALIGNMENT_MASK)
#define POOL_OVERHEAD           ROUNDUP(sizeof(struct PoolHeader))

#define ARENA_SIZE              (256 << 10)

#define POOL_ADDR(P)\
  ((PoolHeaderPtr)((uintptr_t)(P) & ~(uintptr_t)POOL_SIZE_MASK))

typedef uint8_t Block;

typedef struct PoolHeader {
  union { Block* padding; uint32_t count; } ref;
  Block* freeblock;
  struct PoolHeader* prevpool;
  struct PoolHeader* nextpool;
  uint32_t arena_inedx;
  uint32_t size_index;
  uint32_t next_offset;
  uint32_t max_next_offset;
} PoolHeader, *PoolHeaderPtr;

struct ArenaObject {
  uintptr_t address;
  Block* pool_address;
  uint32_t nfreepools;
  uint32_t ntotalpools;
  PoolHeaderPtr freepools;
  struct ArenaObject* next_arena;
  struct ArenaObject* prev_arena;
};

void* pymem_alloc(size_t bytes) {
  PoolHeaderPtr pool;
  Block* bp;

  uint32_t size = SIZE2INDEX(bytes);

  if (pool != pool->nextpool) {
    ++pool->ref.count;
    bp = pool->freeblock;
    if ((pool->freeblock = *(Block**)bp) != NULL)
      return bp;

    if (pool->next_offset <= pool->max_next_offset) {
      pool->freeblock = (Block*)pool + pool->next_offset;
      pool->next_offset += SIZE2INDEX(size);
      *(Block**)(pool->freeblock) = NULL;
      return bp;
    }
  }

  // TODO: need some others operations

  pool->ref.count = 1;
  pool->size_index = size;
  size = SIZE2INDEX(size);
  bp = (Block*)pool + POOL_OVERHEAD;
  pool->next_offset = POOL_OVERHEAD + (size << 1);
  pool->max_next_offset = POOL_SIZE - size;
  pool->freeblock = bp + size;
  *(Block**)(pool->freeblock) = NULL;
  return bp;

  // TODO: need some others operations
}

void pymem_dealloc(void* p) {
  PoolHeaderPtr pool;
  Block* lastfree;

  pool = POOL_ADDR(p);
  if (1) { // TODO: need updated the condition
    *(Block**)p = lastfree = pool->freeblock;
    pool->freeblock = (Block*)p;

    // TODO:
  }
}
