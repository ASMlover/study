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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "njmem.h"

#define ALIGNMENT       (8) /* must be 2^N */
#define ALIGNMENT_SHIFT (3)
#define INDEX2BYTES(I)  (((Nj_size_t)(I) + 1) << ALIGNMENT_SHIFT)
#define BYTES2INDEX(S)  (((Nj_size_t)(S) - 1) >> ALIGNMENT_SHIFT)

#define SMALL_REQUEST_THRESHOLD (512)
#define NB_SMALL_SIZE_CLASSES   (SMALL_REQUEST_THRESHOLD / ALIGNMENT)

#define PAGE_SIZE       (4 << 10)
#define PAGE_SIZE_MASK  (PAGE_SIZE - 1)
#define CHUNK_SIZE      (256 << 10)

typedef struct _NjBlock {
  struct _NjBlock* nextblock;
} NjBlock;

typedef struct _NjChunk {
  struct _NjChunk* nextchunk;
  NjBlock* blockptr;
} NjChunk;

static NjBlock* freeblocks[NB_SMALL_SIZE_CLASSES];
static NjChunk* chunks = NULL;

static void
_insert_chunk(NjBlock* blockptr) {
  NjChunk* chunk = (NjChunk*)malloc(sizeof(NjChunk));
  assert(chunk != NULL);

  chunk->blockptr = blockptr;
  chunk->nextchunk = chunks;
  chunks = chunk;
}

static NjBlock*
_alloc_chunk(Nj_size_t index) {
  Nj_size_t block_bytes = INDEX2BYTES(index);
  NjBlock* new_chunk;

  if (freeblocks[index] == NULL) {
    new_chunk = (NjBlock*)malloc(CHUNK_SIZE);
    if (new_chunk == NULL)
      return NULL;
    _insert_chunk(new_chunk);

    Nj_size_t excess = (Nj_uintptr_t)new_chunk & PAGE_SIZE_MASK;
    if (excess != 0)
      freeblocks[index] = (NjBlock*)((Nj_uchar_t*)new_chunk + PAGE_SIZE - excess);

    NjBlock* block = freeblocks[index];
    for (Nj_size_t i = 0; i < CHUNK_SIZE - (PAGE_SIZE - excess + block_bytes); i += block_bytes)
      block = block->nextblock = block + block_bytes / sizeof(NjBlock);
    block->nextblock = NULL;
  }

  return freeblocks[index];
}

void*
njmem_malloc(Nj_size_t bytes) {
  void* r;

  if (bytes <= SMALL_REQUEST_THRESHOLD) {
    Nj_size_t index = BYTES2INDEX(bytes);
    if (freeblocks[index] == NULL)
      _alloc_chunk(index);

    r = freeblocks[index];
    freeblocks[index] = freeblocks[index]->nextblock;
  }
  else {
    r = malloc(bytes);
  }

  return r;
}

void
njmem_free(void* p, Nj_size_t bytes) {
  if (bytes <= SMALL_REQUEST_THRESHOLD) {
    Nj_size_t index = BYTES2INDEX(bytes);
    NjBlock* block = freeblocks[index];
    block->nextblock = freeblocks[index];
    freeblocks[index] = block;
  }
  else {
    free(p);
  }
}

void
njmem_collect(void) {
  while (chunks != NULL) {
    NjChunk* chunk = chunks;
    chunks = chunk->nextchunk;
    free(chunk->blockptr);
    free(chunk);
  }
  memset(freeblocks, 0, sizeof(freeblocks));
}
