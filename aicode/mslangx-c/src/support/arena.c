#include "ms/arena.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct MsArenaBlock {
  struct MsArenaBlock *next;
  size_t capacity;
  size_t used;
  unsigned char data[1];
} MsArenaBlock;

static size_t ms_arena_default_block_size(void) {
  return 4096;
}

static int ms_arena_alignment_is_valid(size_t alignment) {
  return alignment != 0 && (alignment & (alignment - 1)) == 0;
}

static size_t ms_arena_align_up(size_t value, size_t alignment) {
  size_t mask = alignment - 1;

  return (value + mask) & ~mask;
}

static MsArenaBlock *ms_arena_new_block(size_t capacity) {
  MsArenaBlock *block;
  size_t size;

  if (capacity == 0) {
    return NULL;
  }
  if (capacity > SIZE_MAX - sizeof(*block) + 1) {
    return NULL;
  }

  size = sizeof(*block) + capacity - 1;
  block = malloc(size);
  if (!block) {
    return NULL;
  }

  block->next = NULL;
  block->capacity = capacity;
  block->used = 0;
  return block;
}

void ms_arena_init(MsArena *arena, size_t block_size) {
  arena->blocks = NULL;
  arena->block_size = block_size ? block_size : ms_arena_default_block_size();
  arena->total_bytes = 0;
}

void *ms_arena_alloc(MsArena *arena, size_t size, size_t alignment) {
  MsArenaBlock *block;
  size_t needed;
  size_t aligned_used;
  unsigned char *result;

  if (!arena || !ms_arena_alignment_is_valid(alignment)) {
    return NULL;
  }
  if (size == 0) {
    size = 1;
  }
  if (size > SIZE_MAX - alignment) {
    return NULL;
  }
  if (size > SIZE_MAX - arena->total_bytes) {
    return NULL;
  }

  block = arena->blocks;
  needed = size + alignment - 1;
  if (!block || block->capacity - block->used < needed) {
    size_t capacity = arena->block_size;
    MsArenaBlock *new_block;

    if (capacity < needed) {
      capacity = needed;
    }
    new_block = ms_arena_new_block(capacity);
    if (!new_block) {
      return NULL;
    }
    new_block->next = block;
    arena->blocks = new_block;
    block = new_block;
  }

  aligned_used = ms_arena_align_up(block->used, alignment);
  if (aligned_used > block->capacity || size > block->capacity - aligned_used) {
    return NULL;
  }

  result = block->data + aligned_used;
  block->used = aligned_used + size;
  arena->total_bytes += size;
  return result;
}

size_t ms_arena_total_bytes(const MsArena *arena) {
  return arena ? arena->total_bytes : 0;
}

void ms_arena_reset(MsArena *arena) {
  MsArenaBlock *block;

  if (!arena) {
    return;
  }

  block = arena->blocks;
  while (block) {
    MsArenaBlock *next = block->next;

    free(block);
    block = next;
  }

  arena->blocks = NULL;
  arena->total_bytes = 0;
}

void ms_arena_destroy(MsArena *arena) {
  if (!arena) {
    return;
  }

  ms_arena_reset(arena);
  arena->block_size = 0;
}