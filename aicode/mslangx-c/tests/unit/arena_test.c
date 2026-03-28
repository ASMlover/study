#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ms/arena.h"

#include "test_assert.h"

int main(void) {
  enum { kAllocations = 128 };
  MsArena arena;
  uint8_t *blocks[kAllocations];
  size_t i;

  ms_arena_init(&arena, 64);

  for (i = 0; i < kAllocations; ++i) {
    blocks[i] = ms_arena_alloc(&arena, 24, 8);
    TEST_ASSERT(blocks[i] != NULL);
    memset(blocks[i], (int)(i + 1), 24);
  }

  for (i = 1; i < kAllocations; ++i) {
    TEST_ASSERT(blocks[i] != blocks[i - 1]);
  }

  TEST_ASSERT(ms_arena_total_bytes(&arena) >= kAllocations * 24);

  ms_arena_reset(&arena);

  TEST_ASSERT(ms_arena_total_bytes(&arena) == 0);
  TEST_ASSERT(ms_arena_alloc(&arena, 32, 8) != NULL);

  ms_arena_destroy(&arena);
  return 0;
}