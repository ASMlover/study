#ifndef MSLANGC_TABLE_H_
#define MSLANGC_TABLE_H_

#include <stddef.h>
#include <stdint.h>

#include "ms/string.h"
#include "ms/value.h"

typedef struct MsTableEntry {
  MsString *key;
  MsValue value;
} MsTableEntry;

typedef struct MsTable {
  size_t count;
  size_t capacity;
  MsTableEntry *entries;
} MsTable;

/* Tables do not own keys or values. Use ms_table_find_string() for interning. */
void ms_table_init(MsTable *table);
void ms_table_destroy(MsTable *table);
size_t ms_table_count(const MsTable *table);
size_t ms_table_capacity(const MsTable *table);
int ms_table_set(MsTable *table,
                 MsString *key,
                 MsValue value,
                 int *out_inserted_new);
int ms_table_get(const MsTable *table,
                 const MsString *key,
                 MsValue *out_value,
                 int *out_found);
int ms_table_delete(MsTable *table, const MsString *key, int *out_deleted);
MsString *ms_table_find_string(const MsTable *table,
                               const char *bytes,
                               size_t length,
                               uint32_t hash);

#endif