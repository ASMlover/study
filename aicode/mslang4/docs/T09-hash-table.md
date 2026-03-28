# T09: Hash Table

**Phase**: 2 - Core Data Types
**Dependencies**: T07 (Value System), T08 (Object System - Strings)
**Estimated Complexity**: High

## Goal

Implement the hash table data structure used for globals, string interning, class methods, instance fields, and module exports. Open addressing with linear probing, using tombstones for deletion.

## Files to Create

| File | Purpose |
|------|---------|
| `src/table.h` | Table types and API |
| `src/table.c` | Table implementation |

## TDD Implementation Cycles

### Cycle 1: Table Init and Free

**RED** — Write failing test:

Create `tests/unit/test_table.c`. Write a test function `test_table_init_free` that:
- Initializes a `MsTable`
- Verifies `entries == NULL`, `count == 0`, `capacity == 0`
- Frees the table and verifies no crash

```c
// tests/unit/test_table.c (initial skeleton)
#include "table.h"
#include "object.h"
#include "value.h"
#include <stdio.h>
#include <assert.h>

static void test_table_init_free(void) {
    MsTable table;
    ms_table_init(&table);
    assert(table.entries == NULL);
    assert(table.count == 0);
    assert(table.capacity == 0);

    ms_table_free(&table);
    assert(table.entries == NULL);
    assert(table.count == 0);
    assert(table.capacity == 0);
    printf("  test_table_init_free PASSED\n");
}

int main(void) {
    printf("Running table tests...\n");
    test_table_init_free();
    printf("All table tests passed.\n");
    return 0;
}
```

**Verify RED**: `gcc -I src -o build/test_table tests/unit/test_table.c src/table.c src/object.c src/value.c` → compilation error: `table.h: No such file or directory`

**GREEN** — Minimal implementation:

Create `src/table.h`:

```c
#ifndef MS_TABLE_H
#define MS_TABLE_H

#include "value.h"
#include "object.h"

#define MS_TABLE_MAX_LOAD 0.75

typedef struct { MsString* key; MsValue value; } MsTableEntry;
typedef struct { MsTableEntry* entries; int count; int capacity; } MsTable;

void ms_table_init(MsTable* table);
void ms_table_free(MsTable* table);
bool ms_table_set(MsTable* table, MsString* key, MsValue value);
bool ms_table_get(MsTable* table, MsString* key, MsValue* outValue);
bool ms_table_remove(MsTable* table, MsString* key);
void ms_table_add_all(MsTable* from, MsTable* to);
MsString* ms_table_find_string(MsTable* table, const char* chars, int length, uint32_t hash);
void ms_table_remove_white(MsTable* table);
void ms_table_mark(MsTable* table);

#endif
```

Create `src/table.c` with init and free:

```c
#include "table.h"
#include "common.h"
#include <stdlib.h>

void ms_table_init(MsTable* table) {
    table->entries = NULL;
    table->count = 0;
    table->capacity = 0;
}

void ms_table_free(MsTable* table) {
    MS_FREE_ARRAY(MsTableEntry, table->entries, table->capacity);
    ms_table_init(table);
}
```

**Verify GREEN**: `gcc -I src -o build/test_table tests/unit/test_table.c src/table.c src/object.c src/value.c && ./build/test_table` → test passes

**REFACTOR**: None needed.

---

### Cycle 2: Table Set and Get (Single Entry)

**RED** — Write failing test:

Add `test_table_set_get` to `test_table.c`:

```c
static void test_table_set_get(void) {
    MsTable table;
    ms_table_init(&table);

    MsString* key = ms_string_copy("x", 1);
    MsValue val = ms_number_val(42.0);

    bool is_new = ms_table_set(&table, key, val);
    assert(is_new);
    assert(table.count == 1);

    MsValue out;
    bool found = ms_table_get(&table, key, &out);
    assert(found);
    assert(ms_as_number(out) == 42.0);

    ms_table_free(&table);
    ms_object_free((MsObject*)key);
    printf("  test_table_set_get PASSED\n");
}
```

**Verify RED**: Build fails — `ms_table_set` and `ms_table_get` undefined.

**GREEN** — Minimal implementation:

Add to `src/table.c`:

```c
static MsTableEntry* ms_table_find_entry(MsTableEntry* entries, int capacity, MsString* key) {
    uint32_t index = key->hash % capacity;
    MsTableEntry* tombstone = NULL;
    for (;;) {
        MsTableEntry* entry = &entries[index];
        if (entry->key == NULL) {
            if (MS_IS_NIL(entry->value)) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }
        index = (index + 1) % capacity;
    }
}
```

Note: Use a sentinel check for tombstones. The tombstone sentinel is `key == NULL && value is not nil`. We need a way to detect this. Use `MS_IS_NIL` to distinguish empty from tombstone.

Implement `ms_table_set` and `ms_table_get`:

```c
static void ms_table_adjust_capacity(MsTable* table, int new_capacity) {
    MsTableEntry* new_entries = MS_ALLOCATE(MsTableEntry, new_capacity);
    for (int i = 0; i < new_capacity; i++) {
        new_entries[i].key = NULL;
        new_entries[i].value = ms_nil_val();
    }
    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        MsTableEntry* entry = &table->entries[i];
        if (entry->key == NULL) continue;
        MsTableEntry* dest = ms_table_find_entry(new_entries, new_capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }
    MS_FREE_ARRAY(MsTableEntry, table->entries, table->capacity);
    table->entries = new_entries;
    table->capacity = new_capacity;
}

bool ms_table_set(MsTable* table, MsString* key, MsValue value) {
    if (table->count + 1 > table->capacity * MS_TABLE_MAX_LOAD) {
        int capacity = MS_GROW_CAPACITY(table->capacity);
        ms_table_adjust_capacity(table, capacity);
    }
    MsTableEntry* entry = ms_table_find_entry(table->entries, table->capacity, key);
    bool is_new_key = entry->key == NULL;
    if (is_new_key && ms_is_nil(entry->value)) table->count++;
    entry->key = key;
    entry->value = value;
    return is_new_key;
}

bool ms_table_get(MsTable* table, MsString* key, MsValue* outValue) {
    if (table->count == 0) return false;
    MsTableEntry* entry = ms_table_find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;
    *outValue = entry->value;
    return true;
}
```

Key logic:
1. If load factor `count+1 > capacity * MS_TABLE_MAX_LOAD`, grow to `MS_GROW_CAPACITY(capacity)`
2. When growing, rehash all entries (skip NULL keys and tombstones)
3. Find bucket: `index = key->hash % capacity`, linear probe
4. If found existing key → update value, return false (not new)
5. If empty or tombstone → insert, increment count, return true
6. For get: probe using hash. If key matches → set outValue, return true. If empty entry → return false.

**Verify GREEN**: `gcc -I src -o build/test_table tests/unit/test_table.c src/table.c src/object.c src/value.c && ./build/test_table` → both tests pass

**REFACTOR**: None needed.

---

### Cycle 3: Table Set Overwrite (Existing Key)

**RED** — Write failing test:

Add `test_table_overwrite` to `test_table.c`:

```c
static void test_table_overwrite(void) {
    MsTable table;
    ms_table_init(&table);

    MsString* key = ms_string_copy("x", 1);

    bool is_new1 = ms_table_set(&table, key, ms_number_val(1.0));
    assert(is_new1 == true);

    bool is_new2 = ms_table_set(&table, key, ms_number_val(2.0));
    assert(is_new2 == false);
    assert(table.count == 1);

    MsValue out;
    bool found = ms_table_get(&table, key, &out);
    assert(found);
    assert(ms_as_number(out) == 2.0);

    ms_table_free(&table);
    ms_object_free((MsObject*)key);
    printf("  test_table_overwrite PASSED\n");
}
```

**Verify RED**: Should compile — `ms_table_set` already returns false for existing keys. If the implementation from Cycle 2 is correct, this test will pass immediately.

**Verify GREEN**: Build and run — all three tests pass.

**REFACTOR**: None needed.

---

### Cycle 4: Table Remove and Tombstones

**RED** — Write failing test:

Add `test_table_remove_tombstone` to `test_table.c`:

```c
static void test_table_remove_tombstone(void) {
    MsTable table;
    ms_table_init(&table);

    MsString* keyA = ms_string_copy("a", 1);
    MsString* keyB = ms_string_copy("b", 1);
    MsString* keyC = ms_string_copy("c", 1);

    ms_table_set(&table, keyA, ms_number_val(1.0));
    ms_table_set(&table, keyB, ms_number_val(2.0));
    ms_table_set(&table, keyC, ms_number_val(3.0));

    /* Remove B (creates tombstone) */
    bool removed = ms_table_remove(&table, keyB);
    assert(removed);

    /* Get B should fail */
    MsValue out;
    assert(!ms_table_get(&table, keyB, &out));

    /* Get A and C should still work (tombstones don't break lookups) */
    assert(ms_table_get(&table, keyA, &out));
    assert(ms_as_number(out) == 1.0);
    assert(ms_table_get(&table, keyC, &out));
    assert(ms_as_number(out) == 3.0);

    /* Remove non-existent key */
    MsString* keyD = ms_string_copy("d", 1);
    assert(!ms_table_remove(&table, keyD));

    ms_table_free(&table);
    ms_object_free((MsObject*)keyA);
    ms_object_free((MsObject*)keyB);
    ms_object_free((MsObject*)keyC);
    ms_object_free((MsObject*)keyD);
    printf("  test_table_remove_tombstone PASSED\n");
}
```

**Verify RED**: Build fails — `ms_table_remove` undefined.

**GREEN** — Minimal implementation:

Add to `src/table.c`:

```c
bool ms_table_remove(MsTable* table, MsString* key) {
    if (table->count == 0) return false;
    MsTableEntry* entry = ms_table_find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;
    /* Tombstone: key=NULL, value=non-nil (e.g., ms_bool_val(true)) */
    entry->key = NULL;
    entry->value = ms_bool_val(true);
    return true;
}
```

Replace with tombstone: `key=NULL`, `value=ms_bool_val(true)` (non-nil sentinel). Return true if found, false otherwise.

**Verify GREEN**: Build and run — all four tests pass.

**REFACTOR**: None needed.

---

### Cycle 5: Table Growth (Dynamic Resizing)

**RED** — Write failing test:

Add `test_table_growth` to `test_table.c`:

```c
static void test_table_growth(void) {
    MsTable table;
    ms_table_init(&table);

    MsString* keys[200];
    for (int i = 0; i < 200; i++) {
        char buf[16];
        int len = snprintf(buf, sizeof(buf), "key_%d", i);
        keys[i] = ms_string_copy(buf, len);
        ms_table_set(&table, keys[i], ms_number_val((double)i));
    }

    assert(table.count == 200);

    for (int i = 0; i < 200; i++) {
        MsValue out;
        bool found = ms_table_get(&table, keys[i], &out);
        assert(found);
        assert(ms_as_number(out) == (double)i);
    }

    ms_table_free(&table);
    for (int i = 0; i < 200; i++) {
        ms_object_free((MsObject*)keys[i]);
    }
    printf("  test_table_growth PASSED\n");
}
```

**Verify RED**: Should compile — growth was implemented in Cycle 2's `ms_table_adjust_capacity`. This is a stress/verification test. If growth is broken, it will fail at runtime.

**Verify GREEN**: `gcc -I src -o build/test_table tests/unit/test_table.c src/table.c src/object.c src/value.c && ./build/test_table` → all five tests pass.

**REFACTOR**: Run with address sanitizer to verify no leaks.

---

### Cycle 6: find_string

**RED** — Write failing test:

Add `test_find_string` to `test_table.c`:

```c
static void test_find_string(void) {
    MsTable table;
    ms_table_init(&table);

    MsString* s1 = ms_string_copy("hello", 5);
    MsString* s2 = ms_string_copy("world", 5);
    ms_table_set(&table, s1, ms_number_val(1.0));
    ms_table_set(&table, s2, ms_number_val(2.0));

    /* Find by raw chars+length+hash */
    uint32_t hash = ms_string_hash("hello", 5);
    MsString* found = ms_table_find_string(&table, "hello", 5, hash);
    assert(found == s1);

    /* Not found */
    MsString* not_found = ms_table_find_string(&table, "nope", 4, ms_string_hash("nope", 4));
    assert(not_found == NULL);

    ms_table_free(&table);
    ms_object_free((MsObject*)s1);
    ms_object_free((MsObject*)s2);
    printf("  test_find_string PASSED\n");
}
```

**Verify RED**: Build fails — `ms_table_find_string` undefined.

**GREEN** — Minimal implementation:

Add to `src/table.c`:

```c
MsString* ms_table_find_string(MsTable* table, const char* chars, int length, uint32_t hash) {
    if (table->count == 0) return NULL;
    uint32_t index = hash % table->capacity;
    for (;;) {
        MsTableEntry* entry = &table->entries[index];
        if (entry->key == NULL) {
            if (ms_is_nil(entry->value)) return NULL;
        } else if (entry->key->length == length &&
                   entry->key->hash == hash &&
                   memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }
        index = (index + 1) % table->capacity;
    }
}
```

Iterate from `hash % capacity`, compare length → hash → chars. Return matching string or NULL. Skip tombstones (key=NULL, value non-nil).

**Verify GREEN**: Build and run — all six tests pass.

**REFACTOR**: None needed.

---

### Cycle 7: add_all

**RED** — Write failing test:

Add `test_add_all` to `test_table.c`:

```c
static void test_add_all(void) {
    MsTable src, dst;
    ms_table_init(&src);
    ms_table_init(&dst);

    MsString* k1 = ms_string_copy("a", 1);
    MsString* k2 = ms_string_copy("b", 1);
    ms_table_set(&src, k1, ms_number_val(10.0));
    ms_table_set(&src, k2, ms_number_val(20.0));

    ms_table_add_all(&src, &dst);
    assert(dst.count == 2);

    MsValue out;
    assert(ms_table_get(&dst, k1, &out));
    assert(ms_as_number(out) == 10.0);
    assert(ms_table_get(&dst, k2, &out));
    assert(ms_as_number(out) == 20.0);

    ms_table_free(&src);
    ms_table_free(&dst);
    ms_object_free((MsObject*)k1);
    ms_object_free((MsObject*)k2);
    printf("  test_add_all PASSED\n");
}
```

**Verify RED**: Build fails — `ms_table_add_all` undefined.

**GREEN** — Minimal implementation:

Add to `src/table.c`:

```c
void ms_table_add_all(MsTable* from, MsTable* to) {
    for (int i = 0; i < from->capacity; i++) {
        MsTableEntry* entry = &from->entries[i];
        if (entry->key != NULL) {
            ms_table_set(to, entry->key, entry->value);
        }
    }
}
```

Iterate source entries, skip NULL keys and tombstones, `ms_table_set` into destination.

**Verify GREEN**: Build and run — all seven tests pass.

**REFACTOR**: None needed.

---

### Cycle 8: GC Helpers (remove_white and mark)

**RED** — Write failing test:

Add `test_gc_helpers` to `test_table.c`:

```c
static void test_gc_helpers(void) {
    MsTable table;
    ms_table_init(&table);

    MsString* k1 = ms_string_copy("keep", 4);
    MsString* k2 = ms_string_copy("remove", 6);
    k1->base.isMarked = true;
    k2->base.isMarked = false;

    ms_table_set(&table, k1, ms_number_val(1.0));
    ms_table_set(&table, k2, ms_number_val(2.0));

    /* remove_white should delete entries with unmarked keys */
    ms_table_remove_white(&table);
    assert(table.count == 1);

    MsValue out;
    assert(ms_table_get(&table, k1, &out));
    assert(!ms_table_get(&table, k2, &out));

    /* mark should mark all keys (just verifying no crash for now) */
    ms_table_mark(&table);

    ms_table_free(&table);
    ms_object_free((MsObject*)k1);
    ms_object_free((MsObject*)k2);
    printf("  test_gc_helpers PASSED\n");
}
```

Note: `ms_table_mark` requires a VM parameter for GC marking in the real implementation. For now, test that it doesn't crash. The actual GC integration is in T17.

**Verify RED**: Build fails — `ms_table_remove_white` and `ms_table_mark` undefined.

**GREEN** — Minimal implementation:

Add to `src/table.c`:

```c
void ms_table_remove_white(MsTable* table) {
    for (int i = 0; i < table->capacity; i++) {
        MsTableEntry* entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->base.isMarked) {
            ms_table_remove(table, entry->key);
        }
    }
}

void ms_table_mark(MsTable* table) {
    for (int i = 0; i < table->capacity; i++) {
        MsTableEntry* entry = &table->entries[i];
        if (entry->key != NULL) {
            entry->key->base.isMarked = true;
            /* In full implementation: ms_mark_value(entry->value) */
        }
    }
}
```

`remove_white`: iterate entries, delete entries where `key->isMarked` is false.
`mark`: iterate entries, mark each key and value. Note: the real implementation calls GC mark functions; this is a simplified version that just sets `isMarked`.

**Verify GREEN**: Build and run — all eight tests pass.

**REFACTOR**: `ms_table_mark` will be updated in T17 to use actual GC marking calls instead of directly setting `isMarked`.

## Acceptance Criteria

- [ ] Init/free cycle completes without leak
- [ ] Set then get returns the correct value
- [ ] Set with existing key updates value (returns false)
- [ ] Remove then get returns false
- [ ] Tombstones don't break lookups (set A, set B, remove A, get B)
- [ ] Table grows correctly (insert 100+ entries, all retrievable)
- [ ] `find_string` finds by chars+length+hash
- [ ] `add_all` copies all entries from one table to another
- [ ] `remove_white` removes entries with unmarked keys
- [ ] `mark` marks all keys and values
- [ ] No memory leaks

## Notes

- **Tombstone**: Use a sentinel where `key == NULL` and `value` is non-nil (e.g., `ms_bool_val(true)`). Empty entries have `key == NULL` and `value == ms_nil_val()`.
- **Load factor**: Max 75% (`MS_TABLE_MAX_LOAD`). Grow when exceeded.
- `ms_table_mark` is a simplified placeholder. The real implementation will receive a `MsVM*` or GC context and call proper mark functions (T17).
- All internal helper functions (`ms_table_find_entry`, `ms_table_adjust_capacity`) are `static` in `table.c`.
