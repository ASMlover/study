#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ms/string.h"
#include "ms/table.h"
#include "ms/value.h"

#include "test_assert.h"

static int find_collision_triplet(size_t capacity,
                                  char *first,
                                  char *second,
                                  char *third,
                                  size_t buffer_size) {
  int *first_seen;
  int *second_seen;
  size_t i;

  first_seen = (int *) calloc(capacity, sizeof(*first_seen));
  second_seen = (int *) calloc(capacity, sizeof(*second_seen));
  if (first_seen == NULL || second_seen == NULL) {
    free(first_seen);
    free(second_seen);
    return 0;
  }

  for (i = 0; i < 4096; ++i) {
    char candidate[32];
    uint32_t hash;
    size_t slot;

    snprintf(candidate, sizeof(candidate), "collision-%zu", i);
    hash = ms_string_hash_bytes(candidate, strlen(candidate));
    slot = capacity == 0 ? 0 : (size_t) (hash % capacity);

    if (first_seen[slot] == 0) {
      first_seen[slot] = (int) (i + 1);
      continue;
    }

    if (second_seen[slot] == 0) {
      second_seen[slot] = (int) (i + 1);
      continue;
    }

    snprintf(first, buffer_size, "collision-%d", first_seen[slot] - 1);
    snprintf(second, buffer_size, "collision-%d", second_seen[slot] - 1);
    snprintf(third, buffer_size, "collision-%zu", i);
    free(first_seen);
    free(second_seen);
    return 1;
  }

  free(first_seen);
  free(second_seen);
  return 0;
}

int main(void) {
  MsTable table;
  MsString *seed;
  MsString *first;
  MsString *second;
  MsString *third;
  MsValue stored_value;
  int found = 0;
  int inserted_new = 0;
  int deleted = 0;
  size_t capacity;
  char first_name[32];
  char second_name[32];
  char third_name[32];
  MsString *interned;

  ms_table_init(&table);

  seed = ms_string_from_cstr("seed");
  TEST_ASSERT(seed != NULL);
  TEST_ASSERT(ms_table_set(&table, seed, ms_value_nil(), &inserted_new));
  TEST_ASSERT(inserted_new);

  capacity = ms_table_capacity(&table);
  TEST_ASSERT(capacity > 0);
  TEST_ASSERT(find_collision_triplet(capacity,
                                     first_name,
                                     second_name,
                                     third_name,
                                     sizeof(first_name)));

  first = ms_string_from_cstr(first_name);
  second = ms_string_from_cstr(second_name);
  third = ms_string_from_cstr(third_name);

  TEST_ASSERT(first != NULL);
  TEST_ASSERT(second != NULL);
  TEST_ASSERT(third != NULL);

  TEST_ASSERT(ms_table_set(&table, first, ms_value_number(1.0), &inserted_new));
  TEST_ASSERT(inserted_new);
  TEST_ASSERT(ms_table_set(&table, second, ms_value_number(2.0), &inserted_new));
  TEST_ASSERT(inserted_new);
  TEST_ASSERT(ms_table_count(&table) == 3);

  TEST_ASSERT(ms_table_get(&table, first, &stored_value, &found));
  TEST_ASSERT(found);
  TEST_ASSERT(ms_value_equals(stored_value, ms_value_number(1.0)));

  TEST_ASSERT(ms_table_get(&table, second, &stored_value, &found));
  TEST_ASSERT(found);
  TEST_ASSERT(ms_value_equals(stored_value, ms_value_number(2.0)));

  TEST_ASSERT(ms_table_set(&table, first, ms_value_number(3.0), &inserted_new));
  TEST_ASSERT(!inserted_new);
  TEST_ASSERT(ms_table_get(&table, first, &stored_value, &found));
  TEST_ASSERT(found);
  TEST_ASSERT(ms_value_equals(stored_value, ms_value_number(3.0)));

  TEST_ASSERT(ms_table_delete(&table, first, &deleted));
  TEST_ASSERT(deleted);
  TEST_ASSERT(ms_table_get(&table, first, &stored_value, &found));
  TEST_ASSERT(!found);
  TEST_ASSERT(ms_table_get(&table, second, &stored_value, &found));
  TEST_ASSERT(found);
  TEST_ASSERT(ms_value_equals(stored_value, ms_value_number(2.0)));

  TEST_ASSERT(ms_table_set(&table, third, ms_value_number(4.0), &inserted_new));
  TEST_ASSERT(inserted_new);
  TEST_ASSERT(ms_table_count(&table) == 3);
  TEST_ASSERT(ms_table_get(&table, third, &stored_value, &found));
  TEST_ASSERT(found);
  TEST_ASSERT(ms_value_equals(stored_value, ms_value_number(4.0)));

  interned = ms_table_find_string(&table,
                                  second_name,
                                  strlen(second_name),
                                  ms_string_hash_bytes(second_name,
                                                       strlen(second_name)));
  TEST_ASSERT(interned == second);
  TEST_ASSERT(ms_table_find_string(&table,
                                   "missing",
                                   strlen("missing"),
                                   ms_string_hash_bytes("missing",
                                                        strlen("missing"))) == NULL);

  ms_table_destroy(&table);
  ms_string_free(seed);
  ms_string_free(first);
  ms_string_free(second);
  ms_string_free(third);
  return 0;
}