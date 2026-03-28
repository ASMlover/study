#ifndef MS_ARENA_H_
#define MS_ARENA_H_

#include <stddef.h>

/* Compile-time arena memory must stay separate from the runtime GC heap. */
typedef struct MsArena {
  void *blocks;
  size_t block_size;
  size_t total_bytes;
} MsArena;

void ms_arena_init(MsArena *arena, size_t block_size);
void *ms_arena_alloc(MsArena *arena, size_t size, size_t alignment);
size_t ms_arena_total_bytes(const MsArena *arena);
void ms_arena_reset(MsArena *arena);
void ms_arena_destroy(MsArena *arena);

#endif