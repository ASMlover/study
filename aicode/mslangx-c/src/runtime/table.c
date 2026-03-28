#include "ms/table.h"

#include <stdlib.h>
#include <string.h>

static const double kMsTableMaxLoad = 0.75;

static int ms_table_entry_is_empty(const MsTableEntry *entry) {
  return entry->key == NULL && ms_value_is_nil(entry->value);
}

static int ms_table_keys_equal(const MsString *left, const MsString *right) {
  return ms_string_equals(left, right);
}

static MsTableEntry *ms_table_find_entry(MsTableEntry *entries,
                                         size_t capacity,
                                         const MsString *key) {
  MsTableEntry *tombstone = NULL;
  size_t index;

  index = (size_t) (key->hash % capacity);
  for (;;) {
    MsTableEntry *entry = &entries[index];

    if (entry->key == NULL) {
      if (ms_table_entry_is_empty(entry)) {
        return tombstone != NULL ? tombstone : entry;
      }

      if (tombstone == NULL) {
        tombstone = entry;
      }
    } else if (ms_table_keys_equal(entry->key, key)) {
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

static const MsTableEntry *ms_table_find_entry_const(const MsTableEntry *entries,
                                                     size_t capacity,
                                                     const MsString *key) {
  size_t index;

  index = (size_t) (key->hash % capacity);
  for (;;) {
    const MsTableEntry *entry = &entries[index];

    if (entry->key == NULL) {
      if (ms_table_entry_is_empty(entry)) {
        return NULL;
      }
    } else if (ms_table_keys_equal(entry->key, key)) {
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

static int ms_table_adjust_capacity(MsTable *table, size_t capacity) {
  MsTableEntry *entries;
  size_t i;

  entries = (MsTableEntry *) malloc(capacity * sizeof(*entries));
  if (entries == NULL) {
    return 0;
  }

  for (i = 0; i < capacity; ++i) {
    entries[i].key = NULL;
    entries[i].value = ms_value_nil();
  }

  table->count = 0;
  for (i = 0; i < table->capacity; ++i) {
    MsTableEntry *source = &table->entries[i];
    MsTableEntry *destination;

    if (source->key == NULL) {
      continue;
    }

    destination = ms_table_find_entry(entries, capacity, source->key);
    destination->key = source->key;
    destination->value = source->value;
    table->count += 1;
  }

  free(table->entries);
  table->entries = entries;
  table->capacity = capacity;
  return 1;
}

void ms_table_init(MsTable *table) {
  if (table == NULL) {
    return;
  }

  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void ms_table_destroy(MsTable *table) {
  if (table == NULL) {
    return;
  }

  free(table->entries);
  ms_table_init(table);
}

size_t ms_table_count(const MsTable *table) {
  return table == NULL ? 0 : table->count;
}

size_t ms_table_capacity(const MsTable *table) {
  return table == NULL ? 0 : table->capacity;
}

int ms_table_set(MsTable *table,
                 MsString *key,
                 MsValue value,
                 int *out_inserted_new) {
  MsTableEntry *entry;
  int inserted_new_key;

  if (table == NULL || key == NULL) {
    return 0;
  }

  if ((table->count + 1) > (size_t) (table->capacity * kMsTableMaxLoad)) {
    size_t capacity = table->capacity < 8 ? 8 : table->capacity * 2;

    if (!ms_table_adjust_capacity(table, capacity)) {
      return 0;
    }
  }

  entry = ms_table_find_entry(table->entries, table->capacity, key);
  inserted_new_key = entry->key == NULL;
  if (inserted_new_key) {
    table->count += 1;
    entry->key = key;
  }
  entry->value = value;

  if (out_inserted_new != NULL) {
    *out_inserted_new = inserted_new_key;
  }

  return 1;
}

int ms_table_get(const MsTable *table,
                 const MsString *key,
                 MsValue *out_value,
                 int *out_found) {
  const MsTableEntry *entry;

  if (out_found != NULL) {
    *out_found = 0;
  }
  if (out_value != NULL) {
    *out_value = ms_value_nil();
  }

  if (table == NULL || key == NULL || table->count == 0 || table->capacity == 0) {
    return table != NULL;
  }

  entry = ms_table_find_entry_const(table->entries, table->capacity, key);
  if (entry == NULL) {
    return 1;
  }

  if (out_found != NULL) {
    *out_found = 1;
  }
  if (out_value != NULL) {
    *out_value = entry->value;
  }
  return 1;
}

int ms_table_delete(MsTable *table, const MsString *key, int *out_deleted) {
  MsTableEntry *entry;

  if (out_deleted != NULL) {
    *out_deleted = 0;
  }

  if (table == NULL || key == NULL || table->count == 0 || table->capacity == 0) {
    return table != NULL;
  }

  entry = ms_table_find_entry(table->entries, table->capacity, key);
  if (entry->key == NULL) {
    return 1;
  }

  entry->key = NULL;
  entry->value = ms_value_bool(1);
  table->count -= 1;

  if (out_deleted != NULL) {
    *out_deleted = 1;
  }

  return 1;
}

MsString *ms_table_find_string(const MsTable *table,
                               const char *bytes,
                               size_t length,
                               uint32_t hash) {
  size_t index;

  if (table == NULL || bytes == NULL || table->count == 0 || table->capacity == 0) {
    return NULL;
  }

  index = (size_t) (hash % table->capacity);
  for (;;) {
    const MsTableEntry *entry = &table->entries[index];

    if (entry->key == NULL) {
      if (ms_table_entry_is_empty(entry)) {
        return NULL;
      }
    } else if (entry->key->hash == hash && entry->key->length == length &&
               memcmp(entry->key->bytes, bytes, length) == 0) {
      return entry->key;
    }

    index = (index + 1) % table->capacity;
  }
}