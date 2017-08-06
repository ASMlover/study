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
#define DUMMY_SIZE_INDEX        0xFFFF

#define ARENA_SIZE              (256 << 10)
#define INIT_ARENA_OBJECTS      16

#define POOL_ADDR(P)\
  ((PoolHeaderPtr)((uintptr_t)(P) & ~(uintptr_t)POOL_SIZE_MASK))

#define ADDR_IN_RANGE(P, POOL)\
  ((arenaindex_temp = (POOL)->arena_inedx) < maxarenas &&\
   (uintptr_t)(P) - arenas[arenaindex_temp].address < (uintptr_t)ARENA_SIZE &&\
   arenas[arenaindex_temp].address != 0)

typedef uint8_t Block;

typedef struct PoolHeader {
  union { Block* padding; uint32_t count; } ref;
  Block* freeblock;
  struct PoolHeader* nextpool;
  struct PoolHeader* prevpool;
  uint32_t arena_inedx;
  uint32_t size_index;
  uint32_t next_offset;
  uint32_t max_next_offset;
} PoolHeader, *PoolHeaderPtr;

typedef struct ArenaObject {
  uintptr_t address;
  Block* pool_address;
  uint32_t nfreepools;
  uint32_t ntotalpools;
  PoolHeaderPtr freepools;
  struct ArenaObject* next_arena;
  struct ArenaObject* prev_arena;
} ArenaObject;

static ArenaObject* arenas = NULL;
static uint32_t maxarenas = 0;
static ArenaObject* unused_arenas = NULL;
static ArenaObject* usable_arenas = NULL;
static size_t narenas_currently_allocated = 0;

#define PTA(x)\
  ((PoolHeaderPtr)((uint8_t*)&(usedpools[2 * (x)]) - 2 * sizeof(Block*)))
#define PT(x)   PTA(x), PTA(x)

static PoolHeaderPtr usedpools[2 * ((NB_SMALL_SIZE_CLASSES + 7) / 8) * 8] = {
    PT(0), PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7)
#if NB_SMALL_SIZE_CLASSES > 8
    , PT(8), PT(9), PT(10), PT(11), PT(12), PT(13), PT(14), PT(15)
#if NB_SMALL_SIZE_CLASSES > 16
    , PT(16), PT(17), PT(18), PT(19), PT(20), PT(21), PT(22), PT(23)
#if NB_SMALL_SIZE_CLASSES > 24
    , PT(24), PT(25), PT(26), PT(27), PT(28), PT(29), PT(30), PT(31)
#if NB_SMALL_SIZE_CLASSES > 32
    , PT(32), PT(33), PT(34), PT(35), PT(36), PT(37), PT(38), PT(39)
#if NB_SMALL_SIZE_CLASSES > 40
    , PT(40), PT(41), PT(42), PT(43), PT(44), PT(45), PT(46), PT(47)
#if NB_SMALL_SIZE_CLASSES > 48
    , PT(48), PT(49), PT(50), PT(51), PT(52), PT(53), PT(54), PT(55)
#if NB_SMALL_SIZE_CLASSES > 56
    , PT(56), PT(57), PT(58), PT(59), PT(60), PT(61), PT(62), PT(63)
#if NB_SMALL_SIZE_CLASSES > 64
#error "NB_SMALL_SIZE_CLASSES should be less than 64"
#endif /* NB_SMALL_SIZE_CLASSES > 64 */
#endif /* NB_SMALL_SIZE_CLASSES > 56 */
#endif /* NB_SMALL_SIZE_CLASSES > 48 */
#endif /* NB_SMALL_SIZE_CLASSES > 40 */
#endif /* NB_SMALL_SIZE_CLASSES > 32 */
#endif /* NB_SMALL_SIZE_CLASSES > 24 */
#endif /* NB_SMALL_SIZE_CLASSES > 16 */
#endif /* NB_SMALL_SIZE_CLASSES >  8 */
};

static ArenaObject* new_arena(void) {
  ArenaObject* arenaobj;
  uint32_t excess;

  if (unused_arenas == NULL) {
    uint32_t numarenas;

    numarenas = maxarenas ? maxarenas << 1 : INIT_ARENA_OBJECTS;
    if (numarenas <= maxarenas)
      return NULL;

    uint32_t nbytes = numarenas * sizeof(*arenas);
    arenaobj = (ArenaObject*)realloc(arenas, nbytes);
    if (arenaobj == NULL)
      return NULL;
    arenas = arenaobj;

    for (uint32_t i = maxarenas; i < numarenas; ++i) {
      arenas[i].address = 0;
      arenas[i].next_arena = i < numarenas - 1 ? &arenas[i + 1] : NULL;
    }
    unused_arenas = &arenas[maxarenas];
    maxarenas = numarenas;
  }

  arenaobj = unused_arenas;
  unused_arenas = arenaobj->next_arena;

  arenaobj->address = (uintptr_t)malloc(ARENA_SIZE);
  ++narenas_currently_allocated;

  arenaobj->freepools = NULL;
  arenaobj->pool_address = (Block*)arenas->address;
  arenaobj->nfreepools = ARENA_SIZE / POOL_SIZE;
  excess = (uint32_t)(arenaobj->address & POOL_SIZE_MASK);
  if (excess != 0) {
    --arenaobj->nfreepools;
    arenaobj->pool_address += POOL_SIZE - excess;
  }
  arenaobj->ntotalpools = arenaobj->nfreepools;

  return arenaobj;
}

void* pymem_alloc(size_t bytes) {
  PoolHeaderPtr pool;
  Block* bp;
  PoolHeaderPtr next;
  uint32_t size;

  if ((bytes - 1) < NB_SMALL_SIZE_CLASSES) {
    size = SIZE2INDEX(bytes);
    pool = usedpools[size + size];

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
      next = pool->nextpool;
      pool = pool->prevpool;
      next->prevpool = pool;
      next->nextpool = next;
      return bp;
    }

    if (usable_arenas == NULL) {
      usable_arenas = new_arena();
      if (usable_arenas == NULL)
        goto __redirect;
      usable_arenas->next_arena = usable_arenas->prev_arena = NULL;
    }
    pool = usable_arenas->freepools;
    if (pool != NULL) {
      usable_arenas->freepools = pool->nextpool;
      --usable_arenas->nfreepools;
      if (usable_arenas->nfreepools == 0) {
        usable_arenas = usable_arenas->next_arena;
        if (usable_arenas != NULL)
          usable_arenas->prev_arena = NULL;
      }
__init_pool:
      next = usedpools[size + size];
      pool->nextpool = next;
      pool->prevpool = next;
      next->nextpool = pool;
      next->prevpool = pool;
      pool->ref.count = 1;
      if (pool->size_index == size) {
        bp = pool->freeblock;
        pool->freeblock = *(Block**)bp;
        return bp;
      }

      pool->size_index = size;
      size = SIZE2INDEX(size);
      bp = (Block*)pool + POOL_OVERHEAD;
      pool->next_offset = POOL_OVERHEAD + (size << 1);
      pool->max_next_offset = POOL_SIZE - size;
      pool->freeblock = bp + size;
      *(Block**)(pool->freeblock) = NULL;
      return bp;
    }

    pool = (PoolHeaderPtr)usable_arenas->pool_address;
    pool->arena_inedx = (uint32_t)(usable_arenas - arenas);
    pool->size_index = DUMMY_SIZE_INDEX;
    usable_arenas->pool_address += POOL_SIZE;
    --usable_arenas->nfreepools;
    if (usable_arenas->nfreepools == 0) {
      usable_arenas = usable_arenas->next_arena;
      if (usable_arenas != NULL)
        usable_arenas->prev_arena = NULL;
    }
    goto __init_pool;
  }

__redirect:
  if (bytes == 0)
    bytes = 1;
  return malloc(bytes);
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
